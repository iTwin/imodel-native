/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/Render.h>
#include <DgnPlatform/DgnTexture.h>
#include <DgnPlatform/DgnMaterial.h>
#include <BRepCore/SolidKernel.h>
#include <set>

BEGIN_BENTLEY_RENDER_PRIMITIVES_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(SurfaceMaterial);
DEFINE_POINTER_SUFFIX_TYPEDEFS(DisplayParams);
DEFINE_POINTER_SUFFIX_TYPEDEFS(DisplayParamsCache);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Triangle);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Mesh);
DEFINE_POINTER_SUFFIX_TYPEDEFS(MeshBuilder);
DEFINE_POINTER_SUFFIX_TYPEDEFS(MeshBuilderKey);
DEFINE_POINTER_SUFFIX_TYPEDEFS(MeshBuilderList);
DEFINE_POINTER_SUFFIX_TYPEDEFS(MeshBuilderSet);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Geometry);
DEFINE_POINTER_SUFFIX_TYPEDEFS(GeometryList);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Polyface);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Strokes);
DEFINE_POINTER_SUFFIX_TYPEDEFS(GeometryCollection);
DEFINE_POINTER_SUFFIX_TYPEDEFS(VertexKey);
DEFINE_POINTER_SUFFIX_TYPEDEFS(TriangleKey);
DEFINE_POINTER_SUFFIX_TYPEDEFS(SharedGeom);
DEFINE_POINTER_SUFFIX_TYPEDEFS(PartGeom);
DEFINE_POINTER_SUFFIX_TYPEDEFS(SolidPrimitiveGeom);
DEFINE_POINTER_SUFFIX_TYPEDEFS(GeometryOptions);
DEFINE_POINTER_SUFFIX_TYPEDEFS(GeometryAccumulator);
DEFINE_POINTER_SUFFIX_TYPEDEFS(ColorTable);
DEFINE_POINTER_SUFFIX_TYPEDEFS(PrimitiveBuilder);
DEFINE_POINTER_SUFFIX_TYPEDEFS(MeshList);
DEFINE_POINTER_SUFFIX_TYPEDEFS(ThematicMeshBuilder);

DEFINE_REF_COUNTED_PTR(DisplayParams);
DEFINE_REF_COUNTED_PTR(Mesh);
DEFINE_REF_COUNTED_PTR(MeshBuilder);
DEFINE_REF_COUNTED_PTR(MeshBuilderList);
DEFINE_REF_COUNTED_PTR(Geometry);
DEFINE_REF_COUNTED_PTR(SharedGeom);
DEFINE_REF_COUNTED_PTR(PartGeom);
DEFINE_REF_COUNTED_PTR(SolidPrimitiveGeom);

typedef bvector<Polyface>               PolyfaceList;
typedef bvector<Strokes>                StrokesList;
typedef bvector<Render::MeshPolyline>   PolylineList;
typedef bmap<double, PolyfaceList>      PolyfaceMap;

//=======================================================================================
//! Specifies under what circumstances a GeometryAccumulator should generate normals.
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
enum class NormalMode
{
    Never,              //!< Never generate normals
    Always,             //!< Always generate normals
    CurvedSurfacesOnly, //!< Generate normals only for curved surfaces
};

//=======================================================================================
//! Options for controlling how GeometryAccumulator generates geometry.
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct GeometryOptions
{
    enum class SurfacesOnly { No, Yes }; //!< Yes indicates polylines will not be generated, only meshes.
    enum class GenerateEdges { No, Yes }; //!< Yes indicates edges will be generated for surfaces.

    NormalMode          m_normalMode;
    SurfacesOnly        m_surfaces;
    GenerateEdges       m_generateEdges;

    explicit GeometryOptions(NormalMode normals=NormalMode::Always, SurfacesOnly surfaces=SurfacesOnly::No, GenerateEdges edges=GenerateEdges::Yes)
        : m_normalMode(normals), m_surfaces(surfaces), m_generateEdges(edges) { }
    explicit GeometryOptions(GraphicBuilder::CreateParams const& params, NormalMode normals=NormalMode::Always, SurfacesOnly surfaces=SurfacesOnly::No)
        : GeometryOptions(normals, surfaces, params.IsSceneGraphic() ? GenerateEdges::Yes : GenerateEdges::No) { }

    bool WantSurfacesOnly() const { return SurfacesOnly::Yes == m_surfaces; }
    bool WantEdges() const { return GenerateEdges::Yes == m_generateEdges; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   01/19
//=======================================================================================
enum class ComparePurpose
{
    Merge,  //!< ignores category, subcategory, class, and considers colors equivalent if both have or both lack transparency
    Instance, //!< like merge, but also discriminates on SurfaceMaterial (different UV transforms prohibit batching for instancing)
    Strict  //!< compares all members
};

//=======================================================================================
//! Describes material properties of a surface associated with a DisplayParams.
// @bsistruct                                                   Paul.Connelly   01/19
//=======================================================================================
struct SurfaceMaterial
{
    friend struct DisplayParams;
private:
    MaterialPtr         m_material;
    GradientSymbCPtr    m_gradient;
    TextureMapping      m_textureMapping;

    SurfaceMaterial(MaterialP mat, GradientSymbCP grad, TextureMappingCP tex) : m_material(mat), m_gradient(grad)
        {
        if (nullptr != tex)
            m_textureMapping = *tex;
        }
public:
    SurfaceMaterial() { }
    explicit SurfaceMaterial(TextureMappingCR textureMapping, GradientSymbCP gradient = nullptr) : m_gradient(gradient), m_textureMapping(textureMapping) { }
    DGNPLATFORM_EXPORT SurfaceMaterial(MaterialR material, TextureMapping::Trans2x3 const* transform = nullptr);

    bool IsValid() const { return m_textureMapping.IsValid() || m_material.IsValid(); }
    bool IsEqualTo(SurfaceMaterialCR, ComparePurpose) const;
    bool IsLessThan(SurfaceMaterialCR, ComparePurpose) const;

    MaterialP GetMaterial() const { return m_material.get(); }
    TextureMappingCR GetTextureMapping() const { return m_textureMapping; }
    GradientSymbCP GetGradient() const { return m_gradient.get(); }
};

//=======================================================================================
//! Compares Render::Materials based on the properties used by the iModel.js tile-based renderer.
//! NB: MatchesDefaults returns false if the material has a valid texture mapping - but IsLessThan and
//! Equals ignore the texture mapping entirely (it is intended for use with MaterialAtlas, which does
//! not permit textured materials).
// @bsistruct                                                   Paul.Connelly   07/19
//=======================================================================================
struct MaterialComparison
{
private:
    static Material::CreateParams const& GetDefaults();
public:
    DGNPLATFORM_EXPORT static CompareResult Compare(MaterialP lhs, MaterialP rhs);
    DGNPLATFORM_EXPORT static CompareResult Compare(Material::CreateParams const& lhs, Material::CreateParams const& rhs);

    DGNPLATFORM_EXPORT static bool MatchesDefaults(MaterialP material);
    DGNPLATFORM_EXPORT static bool MatchesDefaults(Material::CreateParams const& params);

    static bool IsLessThan(MaterialP lhs, MaterialP rhs) { return CompareResult::Less == Compare(lhs, rhs); }
    static bool Equals(MaterialP lhs, MaterialP rhs) { return CompareResult::Equal == Compare(lhs, rhs); }
    static bool IsLessThan(Material::CreateParams const& lhs, Material::CreateParams const& rhs) { return CompareResult::Less == Compare(lhs, rhs); }
    static bool Equals(Material::CreateParams const& lhs, Material::CreateParams const& rhs) { return CompareResult::Equal == Compare(lhs, rhs); }
};

//=======================================================================================
//! Describes the appearance of a Geometry.
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct DisplayParams : RefCountedBase
{
    friend struct DisplayParamsCache;

    enum class Type { Mesh, Linear, Text };
    enum class RegionEdgeType { None, Default, Outline };
protected:
    Type                m_type;
    DgnCategoryId       m_categoryId;
    DgnSubCategoryId    m_subCategoryId;
    SurfaceMaterial     m_surfaceMaterial;
    ColorDef            m_lineColor = ColorDef::White(); // all types of geometry (edge color for meshes)
    ColorDef            m_fillColor = ColorDef::White(); // meshes only
    uint32_t            m_width = 0; // linear and mesh (edges)
    LinePixels          m_linePixels = LinePixels::Solid; // linear and mesh (edges)
    FillFlags           m_fillFlags = FillFlags::None; // meshes only
    DgnGeometryClass    m_class = DgnGeometryClass::Primary;
    bool                m_ignoreLighting = false; // always true for text and linear geometry; true for meshes only if normals not desired

    virtual uint32_t _GetExcessiveRefCountThreshold() const override { return 0x7fffffff; }

    DGNPLATFORM_EXPORT DisplayParamsCPtr Clone() const;

    DGNPLATFORM_EXPORT static DisplayParams ForMesh(GraphicParamsCR gf, GeometryParamsCP geom, bool filled, DgnDbR db, System& sys);
    DGNPLATFORM_EXPORT static DisplayParams ForLinear(GraphicParamsCR gf, GeometryParamsCP geom);
    DGNPLATFORM_EXPORT static DisplayParams ForText(GraphicParamsCR gf, GeometryParamsCP geom);
    static DisplayParams ForType(Type type, GraphicParamsCR gf, GeometryParamsCP geom, bool filled, DgnDbR db, System& sys);

    DisplayParams(DisplayParamsCR rhs) = default;
    DisplayParams(ColorDef lineColor, DgnCategoryId catId, DgnSubCategoryId subCatId, DgnGeometryClass geomClass) { InitText(lineColor, catId, subCatId, geomClass); }
    DisplayParams(ColorDef lineColor, uint32_t width, LinePixels px, DgnCategoryId cat, DgnSubCategoryId sub, DgnGeometryClass gc)
        { InitLinear(lineColor, width, px, cat, sub, gc); }
    DisplayParams(ColorDef lineColor, ColorDef fillColor, uint32_t width, LinePixels px, SurfaceMaterialCR mat, FillFlags ff, DgnCategoryId cat, DgnSubCategoryId sub, DgnGeometryClass gc)
        { InitMesh(lineColor, fillColor, width, px, mat, ff, cat, sub, gc); }

    void InitGeomParams(DgnCategoryId, DgnSubCategoryId, DgnGeometryClass);
    void InitText(ColorDef lineColor, DgnCategoryId, DgnSubCategoryId, DgnGeometryClass);
    void InitLinear(ColorDef lineColor, uint32_t width, LinePixels, DgnCategoryId, DgnSubCategoryId, DgnGeometryClass);
    DGNPLATFORM_EXPORT void InitMesh(ColorDef lineColor, ColorDef fillColor, uint32_t width, LinePixels, SurfaceMaterialCR, FillFlags, DgnCategoryId, DgnSubCategoryId, DgnGeometryClass);
public:
    Type GetType() const { return m_type; }
    ColorDef GetFillColorDef() const { return m_fillColor; }
    uint32_t GetFillColor() const { return GetFillColorDef().GetValue(); }
    ColorDef GetLineColorDef() const { return m_lineColor; }
    uint32_t GetLineColor() const { return GetLineColorDef().GetValue(); }

    SurfaceMaterialCR GetSurfaceMaterial() const { return m_surfaceMaterial; }
    bool IsTextured() const { return m_surfaceMaterial.GetTextureMapping().IsValid(); }

    uint32_t GetLineWidth() const { return m_width; }
    LinePixels GetLinePixels() const { return m_linePixels; }
    FillFlags GetFillFlags() const { return m_fillFlags; }

    DgnGeometryClass GetClass() const { return m_class; }
    DgnCategoryId GetCategoryId() const { return m_categoryId; }
    DgnSubCategoryId GetSubCategoryId() const { return m_subCategoryId; }

    bool IgnoresLighting() const { return m_ignoreLighting; }
    bool HasFillTransparency() const { return 0 != GetFillColorDef().GetAlpha(); }
    bool HasLineTransparency() const { return 0 != GetLineColorDef().GetAlpha(); }
    bool HasBlankingFill() const { return FillFlags::Blanking == (GetFillFlags() & FillFlags::Blanking); }

    RegionEdgeType GetRegionEdgeType() const;
    bool WantRegionOutline() const { return RegionEdgeType::Outline == GetRegionEdgeType(); }

    DGNPLATFORM_EXPORT bool IsLessThan(DisplayParamsCR rhs, ComparePurpose purpose=ComparePurpose::Strict) const;
    DGNPLATFORM_EXPORT bool IsEqualTo(DisplayParamsCR rhs, ComparePurpose purpose=ComparePurpose::Strict) const;

    static DisplayParamsCPtr CreateForMesh(GraphicParamsCR gf, GeometryParamsCP geom, bool filled, DgnDbR db, System& sys) { return ForMesh(gf, geom, filled, db, sys).Clone(); }
    static DisplayParamsCPtr CreateForLinear(GraphicParamsCR gf, GeometryParamsCP geom) { return ForLinear(gf, geom).Clone(); }
    static DisplayParamsCPtr CreateForText(GraphicParamsCR gf, GeometryParamsCP geom) { return ForText(gf, geom).Clone(); }
    static DisplayParamsCPtr Create(Type type, DgnCategoryId catId, DgnSubCategoryId subCatId, GradientSymbCP gradient, RenderMaterialId matId, ColorDef lineColor, ColorDef fillColor, uint32_t width, LinePixels linePixels, FillFlags fillFlags, DgnGeometryClass geomClass, bool ignoreLights, DgnDbR dgnDb, System& renderSys, TextureMappingCR texMap);

    DisplayParamsCPtr CloneForRasterText(TextureCR raster) const;
    DisplayParamsCPtr CloneWithTextureOverride(TextureMappingCR textureMapping) const;
    DisplayParamsCPtr CloneForMeshedLineString() const;
    DGNPLATFORM_EXPORT DisplayParamsCPtr Clone(ColorDef fillColor, ColorDef lineColor, uint32_t lineWidth, LinePixels linePixels) const;

    DGNPLATFORM_EXPORT Utf8String ToDebugString() const; //!< @private

    DGNPLATFORM_EXPORT static uint8_t GetMinTransparency();
    DGNPLATFORM_EXPORT static ColorDef AdjustTransparency(ColorDef);
};

//=======================================================================================
//! A cache of ref-counted pointers to DisplayParams objects.
// @bsistruct                                                   Paul.Connelly   05/17
//=======================================================================================
struct DisplayParamsCache
{
private:
    struct Comparator
    {
        bool operator()(DisplayParamsCPtr const& lhs, DisplayParamsCPtr const& rhs) const
            {
            return lhs->IsLessThan(*rhs);
            }
    };

    typedef bset<DisplayParamsCPtr, Comparator> Set;

    Set         m_set;
    DgnDbR      m_db;
    System&     m_system;

    DGNPLATFORM_EXPORT DisplayParamsCR Get(DisplayParamsR);
public:
    DisplayParamsCache(DgnDbR db, System& system) : m_db(db), m_system(system) { }

    DisplayParamsCR GetForMesh(GraphicParamsCR gf, GeometryParamsCP geom, bool filled) { return Get(DisplayParams::Type::Mesh, gf, geom, filled); }
    DisplayParamsCR GetForLinear(GraphicParamsCR gf, GeometryParamsCP geom) { return Get(DisplayParams::Type::Linear, gf, geom, false); }
    DisplayParamsCR GetForText(GraphicParamsCR gf, GeometryParamsCP geom) { return Get(DisplayParams::Type::Text, gf, geom, false); }
    DisplayParamsCR Get(DisplayParams::Type type, GraphicParamsCR gf, GeometryParamsCP geom, bool filled)
        {
        DisplayParams ndp = DisplayParams::ForType(type, gf, geom, filled, m_db, m_system);
        return Get(ndp);
        }

    DgnDbR GetDgnDb() const { return m_db; }
    System& GetSystem() const { return m_system; }
};

//=======================================================================================
//! A look-up table of unique colors. Each unique color is mapped to a sequential index.
// @bsistruct                                                   Paul.Connelly   03/17
//=======================================================================================
struct ColorTable
{
    typedef bmap<uint32_t, uint16_t> Map;
protected:
    Map         m_map;
    bool        m_hasAlpha = false;

    static constexpr uint16_t GetMaxIndex() { return 0xffff; }
public:
    DGNPLATFORM_EXPORT uint16_t GetIndex(uint32_t color);

    bool HasTransparency() const { return m_hasAlpha; }
    bool IsUniform() const { return 1 == size(); }

    bool IsFull() const { return m_map.size() >= GetMaxIndex(); }
    uint16_t GetNumIndices() const { return static_cast<uint16_t>(size()); }

    typedef typename Map::const_iterator const_iterator;
    typedef const_iterator iterator;

    const_iterator begin() const { return m_map.begin(); }
    const_iterator end() const { return m_map.end(); }
    size_t size() const { return m_map.size(); }
    bool empty() const { return m_map.empty(); }
    const_iterator find(uint32_t color) const { return m_map.find(color); }

    void ToColorIndex(ColorIndex& index, bvector<uint32_t>& colors, bvector<uint16_t> const& indices) const;
    Map const& GetMap() const { return m_map; }

    bool FindByIndex(ColorDef& color, uint16_t index) const;
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   03/17
//=======================================================================================
struct MeshList : bvector<MeshPtr>
{
    FeatureTable    m_features;

    explicit MeshList(uint32_t maxFeatures=2048*1024) : m_features(DgnModelId(), maxFeatures) { }

    FeatureTableCR  FeatureTable() const { return m_features; }
    FeatureTableR  FeatureTable()  { return m_features; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Triangle
{
    uint32_t    m_indices[3];
    bool        m_singleSided;
    uint8_t     m_edgeFlags[3];

    uint32_t operator[](size_t index) const { BeAssert(index < 3); if(index>2) index=2; return m_indices[index]; }
    uint32_t& operator[](size_t index) { BeAssert(index < 3); if(index>2) index=2; return m_indices[index]; }

    explicit Triangle(bool singleSided=true) : m_singleSided(singleSided) { SetIndices(0, 0, 0); SetEdgeFlags(MeshEdge::Flags::Visible); }
    Triangle(uint32_t indices[3], bool singleSided) : m_singleSided(singleSided) { SetIndices(indices); SetEdgeFlags(MeshEdge::Flags::Visible); }
    Triangle(uint32_t a, uint32_t b, uint32_t c, bool singleSided) : m_singleSided(singleSided) { SetIndices(a, b, c); SetEdgeFlags(MeshEdge::Flags::Visible); }

    void SetIndices(uint32_t indices[3]) { SetIndices(indices[0], indices[1], indices[2]); }
    void SetIndices(uint32_t a, uint32_t b, uint32_t c) { m_indices[0] = a; m_indices[1] = b; m_indices[2] = c; }
    void SetEdgeFlags(MeshEdge::Flags a, MeshEdge::Flags b, MeshEdge::Flags c)  { m_edgeFlags[0] = a; m_edgeFlags[1] = b; m_edgeFlags[2] = c; }
    void SetEdgeFlags(MeshEdge::Flags a) { m_edgeFlags[0] = m_edgeFlags[1]; m_edgeFlags[2] = a; }
    void SetEdgeFlags(bool const* visible) { m_edgeFlags[0] = visible[0] ? MeshEdge::Flags::Visible : MeshEdge::Flags::Invisible;
                                             m_edgeFlags[1] = visible[1] ? MeshEdge::Flags::Visible : MeshEdge::Flags::Invisible;
                                             m_edgeFlags[2] = visible[2] ? MeshEdge::Flags::Visible : MeshEdge::Flags::Invisible; }
    bool GetEdgeVisible(size_t index) const { BeAssert(index < 3); if(index>2) index=2; return m_edgeFlags[index] == MeshEdge::Flags::Visible; }

    bool IsDegenerate() const  { return m_indices[0] == m_indices[1] || m_indices[0] == m_indices[2] || m_indices[1] == m_indices[2]; }
};

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     06/2017
//=======================================================================================
struct TriangleList
{
private:
    bvector<uint8_t>    m_flags;
    bvector<uint32_t>   m_indices;

public:
    size_t Count() const { return m_indices.size() / 3; }
    bool Empty() const { return m_indices.empty(); }
    bvector<uint32_t> const& Indices() const { return m_indices; }
    void AddTriangle(TriangleCR triangle);
    Triangle  GetTriangle(size_t index) const;
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   03/17
//=======================================================================================
struct MeshArgs : TriMeshArgs
{
    bvector<uint32_t>                   m_colorTable;
    bvector<PolylineEdgeArgs::Polyline> m_polylineEdges;
    bvector<uint8_t>                    m_materialIndices;

    template<typename T, typename U> void Set(T& ptr, U const& src) { ptr = (0 != src.size() ? src.data() : nullptr); }
    template<typename T, typename U> void Set(uint32_t& count, T& ptr, U const& src)
        {
        count = static_cast<uint32_t>(src.size());
        Set(ptr, src);
        }

    void Clear();
    bool Init(MeshCR mesh);
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct PolylineArgs : IndexedPolylineArgs
{
    bvector<Polyline>   m_polylines;
    bvector<uint32_t>   m_colorTable;

    bool IsValid() const { return !m_polylines.empty(); }

    void Reset();
    bool Init(MeshCR mesh);
    void FinishInit(MeshCR mesh);
};

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     05/2017
//=======================================================================================
struct MeshGraphicArgs
{
    PolylineArgs    m_polylineArgs;
    MeshArgs        m_meshArgs;
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   07/19
//=======================================================================================
struct MaterialIndex
{
private:
    uint8_t m_index;
    bool    m_valid;
public:
    explicit MaterialIndex(uint8_t index) : m_index(index), m_valid(true) { }
    MaterialIndex() : m_valid(false) { }

    bool IsValid() const { return m_valid; }
    uint8_t Unwrap() const
        {
        BeAssert(IsValid());
        return m_index;
        }
};

//=======================================================================================
//! Holds no more than 255 materials. Materials are compared based on the properties
//! actually used for rendering in iModel.js; if those properties are all the same, the
//! materials use the same material index. Each vertex in the mesh is assigned a material
//! index corresponding to its material's position in the material atlas; the material
//! atlas itself is appended to the VertexTable, with each material's properties encoded
//! into 4 rgba entries.
//! A single atlas may *only* hold opaque *or* translucent materials, not a combination of both.
//! An opaque material atlas can include the "default material" (represented by a NULL MaterialPtr) for
//! meshes that do not define a custom material.
// @bsistruct                                                   Paul.Connelly   07/19
//=======================================================================================
struct MaterialAtlas : RefCountedBase
{
private:
    struct Comparator
    {
        bool operator()(MaterialPtr const& lhs, MaterialPtr const& rhs) const
            {
            return MaterialComparison::IsLessThan(lhs.get(), rhs.get());
            }
    };

    using Map = bmap<MaterialPtr, uint8_t, Comparator>;
    using TransparencyCategory = Material::Transparency::Category;

    Map                     m_materialToIndex;
    uint8_t                 m_maxMaterials;
    TransparencyCategory    m_transparencyCategory;

    MaterialAtlas(MaterialP material, uint8_t maxMaterials) : m_maxMaterials(maxMaterials)
        {
        m_materialToIndex[material] = 0;
        m_transparencyCategory = nullptr != material ? material->GetParams().m_transparency.Categorize() : TransparencyCategory::None;
        }
public:
    static MaterialAtlasPtr Create(MaterialP material, uint8_t maxMaterials = 255)
        {
        if (maxMaterials == 0)
            return nullptr;
        else if (nullptr != material && material->GetTextureMapping().IsValid())
            return nullptr;

        return maxMaterials > 0 ? new MaterialAtlas(material, maxMaterials) : nullptr;
        }

    uint8_t NumMaterials() const { return static_cast<uint8_t>(m_materialToIndex.size()); }
    bool IsFull() const { return NumMaterials() >= m_maxMaterials; }

    TransparencyCategory GetTransparency() const { return m_transparencyCategory; }
    bool IsTranslucent() const { return TransparencyCategory::Translucent == GetTransparency(); }
    bool IsOpaque() const { return TransparencyCategory::Opaque == GetTransparency(); }
    bool OverridesAlpha() const { return TransparencyCategory::None != GetTransparency(); }

    MaterialIndex Find(MaterialP material) const
        {
        auto found = m_materialToIndex.find(material);
        return m_materialToIndex.end() == found ? MaterialIndex() : MaterialIndex(found->second);
        }

    DGNPLATFORM_EXPORT MaterialIndex Insert(MaterialP material);

    DGNPLATFORM_EXPORT bvector<Material::CreateParams> ToList() const;
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Mesh : RefCountedBase
{
    enum class PrimitiveType
    {
        Mesh,
        Polyline,
        Point
    };


private:
    struct Features
    {
        FeatureTableP       m_table;
        bvector<uint32_t>   m_indices;
        uint32_t            m_uniform;
        bool                m_initialized = false;

        explicit Features(FeatureTableP table) : m_table(table) { }

        void Add(FeatureCR, size_t numVerts);
        DGNPLATFORM_EXPORT void ToFeatureIndex(FeatureIndex& index) const;
        void SetIndices(bvector<uint32_t>&& indices);
    };
public:
    struct Materials
    {
    protected:
        bvector<uint8_t>    m_indices;
        uint8_t             m_uniform;
        bool                m_initialized = false;
        bool                m_haveAtlas;
    public:
        explicit Materials(bool haveAtlas) : m_haveAtlas(haveAtlas) { }
        DGNPLATFORM_EXPORT void Add(uint8_t materialIndex, size_t numVerts);
        bvector<uint8_t> const& GetIndices() const { return m_indices; }

        bool IsEmpty() const { return !m_initialized; }
        bool IsUniform() const { return m_initialized && m_indices.empty(); }
        bool IsNonUniform() const { return m_initialized && !m_indices.empty(); }
    };
private:
    DisplayParamsCPtr               m_displayParams;
    TriangleList                    m_triangles;
    PolylineList                    m_polylines;
    QPoint3dList                    m_verts;
    OctEncodedNormalList            m_normals;
    bvector<FPoint2d>               m_uvParams;
    ColorTable                      m_colorTable;
    bvector<uint16_t>               m_colors;
    Features                        m_features;
    Materials                       m_materials;
    PrimitiveType                   m_type;
    mutable MeshEdgesPtr            m_edges;
    bool                            m_is2d;
    bool                            m_isPlanar;
    PolyfaceAuxData::Channels       m_auxChannels;
    size_t                          m_nodeIndex;
    SharedGeomCPtr                  m_sharedGeom;
    MaterialAtlasPtr                m_materialAtlas;

    Mesh(DisplayParamsCR params, FeatureTableP featureTable, PrimitiveType type, DRange3dCR range, bool is2d, bool isPlanar, size_t nodeIndex, MaterialAtlasP materialAtlas)
        : m_displayParams(&params), m_features(featureTable), m_type(type), m_verts(range), m_is2d(is2d), m_isPlanar(isPlanar), m_nodeIndex(nodeIndex), m_materialAtlas(materialAtlas), m_materials(nullptr != materialAtlas) { }

    friend struct MeshBuilder;
    void SetDisplayParams(DisplayParamsCR params) { m_displayParams = &params; }
public:
    static MeshPtr Create(DisplayParamsCR params, FeatureTableP featureTable, PrimitiveType type, DRange3dCR range, bool is2d, bool isPlanar, size_t nodeIndex, MaterialAtlasP materialAtlas)
        { return new Mesh(params, featureTable, type, range, is2d, isPlanar, nodeIndex, materialAtlas); }

    DPoint3d                        GetPoint(uint32_t index) const;
    DGNPLATFORM_EXPORT DRange3d     GetTriangleRange(TriangleCR triangle) const;
    DGNPLATFORM_EXPORT DVec3d       GetTriangleNormal(TriangleCR triangle) const;
    DGNPLATFORM_EXPORT bool         HasNonPlanarNormals() const;

    DisplayParamsCR                 GetDisplayParams() const { return *m_displayParams; } //!< The mesh symbology
    TriangleList const&             Triangles() const { return m_triangles; } //!< Triangles defined as a set of 3 indices into the vertex attribute arrays.
    PolylineList const&             Polylines() const { return m_polylines; } //!< Polylines defined as a set of indices into the vertex attribute arrays.
    PolylineList&                   PolylinesR() { return m_polylines; } //!< Polylines defined as a set of indices into the vertex attribute arrays.
    QPoint3dListCR                  Points() const { return m_verts; } //!< Position vertex attribute array
    QPoint3dListCR                  Verts() const { return m_verts; }
    QPoint3dListR                   VertsR() { return m_verts; }
    OctEncodedNormalListCR          Normals() const { return m_normals; }   //!< Normal vertex attribute array
    OctEncodedNormalListR           NormalsR()  { return m_normals; }       //!< Normal vertex attribute array
    bvector<FPoint2d> const&        Params() const { return m_uvParams; }   //!< UV params vertex attribute array
    bvector<FPoint2d>&              ParamsR() { return m_uvParams; }        //!< UV params vertex attribute array
    bvector<uint16_t> const&        Colors() const { return m_colors; }     //!< Vertex attribute array specifying an index into the color table
    bvector<uint16_t>&              ColorsR() { return m_colors; }          //!< Vertex attribute array specifying an index into the color table
    ColorTableCR                    GetColorTable() const { return m_colorTable; }
    ColorTableR                     GetColorTableR() { return m_colorTable; }
    void                            ToFeatureIndex(FeatureIndex& index) const { m_features.ToFeatureIndex(index); }
    MeshEdgesPtr                    GetEdges() const { return m_edges; }
    MeshEdgesPtr&                   GetEdgesR() { return m_edges; }
    void                            SetFeatureIndices (bvector<uint32_t>&& indices) { m_features.SetIndices(std::move(indices)); }
    bvector<uint32_t> const&        GetFeatureIndices() const { return m_features.m_indices; }
    bool                            GetUniformFeatureIndex(uint32_t& index) const { if (!m_features.m_initialized || !m_features.m_indices.empty()) return false; index = m_features.m_uniform; return true; }
    void                            CompressVertexQuantization();
    size_t                          GetNodeIndex() const { return m_nodeIndex; }

    MaterialAtlasP                  GetMaterialAtlas() const { return m_materialAtlas.get(); }
    uint8_t                         GetMaterialIndex(MaterialP material) const;
    bvector<uint8_t> const&         GetMaterialIndices() const { return m_materials.GetIndices(); }
    bool                            HasMultipleMaterials() const { return m_materials.IsNonUniform(); }

    bool IsEmpty() const { return m_triangles.Empty() && m_polylines.empty(); }
    bool Is2d() const { return m_is2d; }
    bool IsPlanar() const { return m_isPlanar; }
    PrimitiveType GetType() const { return m_type; }
    FeatureTableCP GetFeatureTable() const { return m_features.m_table; }
    PolyfaceAuxData::Channels GetAuxChannels() const { return m_auxChannels; }

    DGNPLATFORM_EXPORT DRange3d ComputeRange() const;
    DGNPLATFORM_EXPORT DRange3d ComputeUVRange() const;

    void AddTriangle(TriangleCR triangle) { BeAssert(PrimitiveType::Mesh == GetType()); m_triangles.AddTriangle(triangle); }
    void AddPolyline(MeshPolylineCR polyline);
    uint32_t AddVertex(QPoint3dCR vertex, OctEncodedNormalCP normal, DPoint2dCP param, uint32_t fillColor, FeatureCR feature, uint8_t materialIndex);
    void AddAuxData(PolyfaceAuxDataCR auxData, size_t index);

    GraphicPtr GetGraphics (MeshGraphicArgs& args, Dgn::Render::SystemCR system, DgnDbR db) const;

    void SetSharedGeom(SharedGeomCR geom) { m_sharedGeom = &geom; }
    SharedGeomCP GetSharedGeom() const { return m_sharedGeom.get(); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   03/17
//=======================================================================================
struct ToleranceRatio
{
    static constexpr double Vertex() { return .1; }
    static constexpr double FacetArea() { return .1; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   07/19
//=======================================================================================
struct MeshBuilderKey
{
    enum class Flags : uint8_t
    {
    None = 0,
    HasNormals = 1 << 0,
    IsPlanar = 1 << 2,
    All = HasNormals | IsPlanar,
    };

private:
    DisplayParamsCPtr   m_params;
    uint64_t            m_nodeIndex;
    Mesh::PrimitiveType m_type;
    Flags               m_flags;

    MeshBuilderKey(DisplayParamsCP params, Mesh::PrimitiveType type, Flags flags, uint64_t nodeIndex = 0) : m_params(params), m_type(type), m_flags(flags), m_nodeIndex(nodeIndex) { }
public:
    MeshBuilderKey() : MeshBuilderKey(nullptr, Mesh::PrimitiveType::Mesh, Flags::None) { }
    MeshBuilderKey(DisplayParamsCR params, Mesh::PrimitiveType type, Flags flags, uint64_t elementId = 0)
        : MeshBuilderKey(&params, type, flags, elementId) { }
    MeshBuilderKey(DisplayParamsCR params, bool hasNormals, Mesh::PrimitiveType type, bool isPlanar, uint64_t elementId = 0)
        : MeshBuilderKey(params, type, MakeFlags(hasNormals, isPlanar), elementId) { }

    DisplayParamsCR GetDisplayParams() const { return *m_params; }
    Mesh::PrimitiveType GetPrimitiveType() const { return m_type; }
    bool IsPlanar() const { return Flags::IsPlanar == m_flags || Flags::All == m_flags; }
    uint64_t GetNodeIndex() const { return m_nodeIndex; }

    bool operator<(MeshBuilderKeyCR rhs) const
        {
        if (m_type != rhs.m_type)
            return m_type < rhs.m_type;

        if (m_flags != rhs.m_flags)
            return m_flags < rhs.m_flags;

        if (m_nodeIndex != rhs.m_nodeIndex)
            return m_nodeIndex < rhs.m_nodeIndex;

        BeAssert(m_params.IsValid() && rhs.m_params.IsValid());
        return m_params->IsLessThan(*rhs.m_params, ComparePurpose::Merge);
        }

    static Flags MakeFlags(bool hasNormals, bool isPlanar)
        {
        if (hasNormals && isPlanar)
            return Flags::All;
        else if (hasNormals)
            return Flags::HasNormals;
        else
            return isPlanar ? Flags::IsPlanar : Flags::None;
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct VertexKey
{
private:
    struct NormalAndPosition
    {
        uint16_t    m_data[4];

        NormalAndPosition() { }
        NormalAndPosition(QPoint3dCR pos, uint16_t normal)
            {
            m_data[0] = normal;
            m_data[1] = pos.x;
            m_data[2] = pos.y;
            m_data[3] = pos.z;
            }
    };

    DPoint2d            m_param;            // 0
    NormalAndPosition   m_normalAndPos;     // 10
    uint64_t            m_elemId;           // 18
    uint64_t            m_subcatId;         // 20
    uint32_t            m_fillColor;        // 28
    DgnGeometryClass    m_class;            // 2C
    uint8_t             m_materialIndex;    // 2D
    bool                m_normalValid;      // 2E
    bool                m_paramValid;       // 2F

    VertexKey(QPoint3dCR point, FeatureCR feature, uint32_t fillColor, uint16_t normal, bool normalValid, bool paramValid, uint8_t materialIndex)
        : m_normalAndPos(point, normal), m_elemId(feature.GetElementId().GetValueUnchecked()), m_subcatId(feature.GetSubCategoryId().GetValueUnchecked()),
        m_fillColor(fillColor), m_class(feature.GetClass()), m_normalValid(normalValid), m_paramValid(paramValid), m_materialIndex(materialIndex) { }
public:
    VertexKey() { }
    VertexKey(DPoint3dCR point, FeatureCR feature, uint32_t fillColor, QPoint3d::ParamsCR qParams, uint8_t materialIndex, DVec3dCP normal=nullptr, DPoint2dCP param=nullptr)
        : VertexKey(QPoint3d(point, qParams), feature, fillColor, nullptr != normal ? OctEncodedNormal::From(*normal).Value() : 0, nullptr != normal, nullptr != param, materialIndex)
        {
        if (m_paramValid)
            m_param = *param;
        }
    VertexKey(QPoint3dCR point, FeatureCR feature, uint32_t fillColor, OctEncodedNormalCP normal, FPoint2dCP param, uint8_t materialIndex)
        : VertexKey(point, feature, fillColor, nullptr != normal ? normal->Value() : 0, nullptr != normal, nullptr != param, materialIndex)
        {
        if (m_paramValid)
            m_param = DPoint2d::From(param->x, param->y);
        }

    DGNPLATFORM_EXPORT bool operator<(VertexKey const& rhs) const;

    QPoint3dCR GetPosition() const { return *reinterpret_cast<QPoint3dCP>(m_normalAndPos.m_data+1); }
    Feature GetFeature() const { return Feature(DgnElementId(m_elemId), DgnSubCategoryId(m_subcatId), m_class); }
    uint32_t GetFillColor() const { return m_fillColor; }
    OctEncodedNormalCP GetNormal() const { return m_normalValid ? reinterpret_cast<OctEncodedNormalCP>(m_normalAndPos.m_data) : nullptr; }
    DPoint2dCP GetParam() const { return m_paramValid ? &m_param : nullptr; }
    uint8_t GetMaterialIndex() const { return m_materialIndex; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct TriangleKey
{
    uint32_t        m_sortedIndices[3];

    TriangleKey() { }
    explicit TriangleKey(TriangleCR triangle);

    bool operator<(TriangleKeyCR rhs) const;
};

typedef bmap<VertexKey, uint32_t> VertexMap;
typedef bset<TriangleKey> TriangleSet;

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct MeshBuilder : RefCountedBase
{
    struct Polyface : RefCountedBase
        {
        PolyfaceQueryCR             m_polyface;
        MeshEdgeCreationOptions     m_edgeOptions;
        size_t                      m_baseTriangleIndex;
        bmap<uint32_t, uint32_t>    m_vertexIndexMap;           // Map from the Mesh vertex index to the polyface vertex index.

        Polyface(PolyfaceQueryCR polyface, MeshEdgeCreationOptionsCR edgeOptions, size_t baseTriangleIndex) : m_polyface(polyface), m_edgeOptions(edgeOptions), m_baseTriangleIndex(baseTriangleIndex) { }
        };

private:
    friend struct MeshBuilderList;

    MeshPtr                         m_mesh;
    VertexMap                       m_vertexMap;
    TriangleSet                     m_triangleSet;
    double                          m_tolerance;
    double                          m_areaTolerance;
    RefCountedPtr<Polyface>         m_currentPolyface;
    DRange3d                        m_tileRange;
    MeshBuilderPtr                  m_next;

    MeshBuilder(DisplayParamsCR params, double tolerance, double areaTolerance, FeatureTableP featureTable, Mesh::PrimitiveType type, DRange3dCR range, bool is2d, bool isPlanar, size_t nodeIndex, MaterialAtlasP materialAtlas)
        : m_mesh(Mesh::Create(params, featureTable, type, range, is2d, isPlanar, nodeIndex, materialAtlas)), m_tolerance(tolerance), m_areaTolerance(areaTolerance), m_tileRange(range) { }

    uint32_t AddVertex(VertexMap& vertices, VertexKeyCR vertex);
public:
    static MeshBuilderPtr Create(DisplayParamsCR params, double tolerance, double areaTolerance, FeatureTableP featureTable, Mesh::PrimitiveType type, DRange3dCR range, bool is2d, bool isPlanar, size_t nodeIndex, MaterialAtlasP materialAtlas)
        { return new MeshBuilder(params, tolerance, areaTolerance, featureTable, type, range, is2d, isPlanar, nodeIndex, materialAtlas); }

    DGNPLATFORM_EXPORT void AddFromPolyfaceVisitor(PolyfaceVisitorR visitor, TextureMappingCR, DgnDbR dgnDb, FeatureCR feature, bool includeParams, uint32_t fillColor, bool requireNormals, uint8_t materialIndex);
    DGNPLATFORM_EXPORT void AddPolyline(bvector<DPoint3d>const& polyline, FeatureCR feature, uint32_t fillColor, double startDistance);
    void AddPolyline(bvector<QPoint3d> const&, FeatureCR, uint32_t fillColor, double startDistance);
    void AddPointString(bvector<DPoint3d> const& pointString, FeatureCR feature, uint32_t fillColor, double startDistance);
    DGNPLATFORM_EXPORT void BeginPolyface(PolyfaceQueryCR polyface, MeshEdgeCreationOptionsCR options);
    DGNPLATFORM_EXPORT void EndPolyface();

    void AddMesh(TriangleCR triangle);
    void AddTriangle(TriangleCR triangle);
    uint32_t AddVertex(VertexKey const& vertex) { return AddVertex(m_vertexMap, vertex); }

    MeshP GetMesh() { return m_mesh.get(); } //!< The mesh under construction
    MeshCP GetMesh() const { return m_mesh.get(); }
    double GetTolerance() const { return m_tolerance; }
    void SetDisplayParams(DisplayParamsCR params) { m_mesh->SetDisplayParams(params); }

    uint8_t GetMaterialIndex(MaterialP material) const { return m_mesh->GetMaterialIndex(material); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   07/19
//=======================================================================================
struct MeshBuilderList : RefCountedBase
{
private:
    MeshBuilderKey      m_key;
    MeshBuilderPtr      m_head;
    MeshBuilderSetCR    m_set;

    DGNPLATFORM_EXPORT MeshBuilderList(MeshBuilderKeyCR key, MeshBuilderSetCR set);

    MeshBuilderPtr CreateMeshBuilder(DisplayParamsCR) const;
public:
    static MeshBuilderListPtr Create(MeshBuilderKeyCR key, MeshBuilderSetCR set) { return new MeshBuilderList(key, set); }

    DGNPLATFORM_EXPORT MeshBuilderR GetMeshBuilder(DisplayParamsCR params, uint32_t numVertices);
    DGNPLATFORM_EXPORT MeshBuilderKeyCR GetKey() const { return m_key; }

    template<typename T> void ForEach(T func) const
        {
        bool stop = false;
        MeshBuilderCPtr list = m_head;
        do
            {
            func(*list, stop);
            if (stop)
                break;

            list = list->m_next;
            }
        while (list.IsValid());
        }

    DGNPLATFORM_EXPORT size_t GetVertexCount() const;
    size_t size() const
        {
        size_t count = 0;
        ForEach([&](MeshBuilderCR, bool&) { ++count; });
        return count;
        }

    void GetMeshes(MeshListR meshes) const;
    void GetMeshes(bvector<MeshPtr>& meshes, SharedGeomCR) const;
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   07/19
//=======================================================================================
struct MeshBuilderKeyComparator
{
    using is_transparent = std::true_type;

    bool operator()(MeshBuilderKeyCR lhs, MeshBuilderKeyCR rhs) const { return lhs < rhs; }
    bool operator()(MeshBuilderListPtr const& lhs, MeshBuilderListPtr const& rhs) const { return lhs->GetKey() < rhs->GetKey(); }
    bool operator()(MeshBuilderListPtr const& lhs, MeshBuilderKeyCR rhs) const { return lhs->GetKey() < rhs; }
    bool operator()(MeshBuilderKeyCR lhs, MeshBuilderListPtr const& rhs) const { return lhs < rhs->GetKey(); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   07/19
//=======================================================================================
struct MeshBuilderSet
{
    DGNPLATFORM_EXPORT static uint32_t GetDefaultMaxVertices();
private:
    using Set = std::set<MeshBuilderListPtr, MeshBuilderKeyComparator>;

    Set m_set;
    double              m_vertexTolerance;
    double              m_facetAreaTolerance;
    DRange3d            m_range;
    FeatureTableP       m_featureTable;
    bool                m_is2d;
    uint8_t             m_maxMaterialsPerMesh;
    uint32_t            m_maxVerticesPerMesh;
public:
    MeshBuilderSet(uint8_t maxMaterialsPerMesh = 255, uint32_t maxVerticesPerMesh = GetDefaultMaxVertices())
        : MeshBuilderSet(0.0, nullptr, DRange3d::NullRange(), false, maxMaterialsPerMesh, maxVerticesPerMesh) { }
    MeshBuilderSet(double tolerance, FeatureTableP features, DRange3dCR range, bool is2d, uint8_t maxMaterialsPerMesh = 255, uint32_t maxVerticesPerMesh = GetDefaultMaxVertices()) :
        m_vertexTolerance(tolerance*ToleranceRatio::Vertex()), m_facetAreaTolerance(tolerance*ToleranceRatio::FacetArea()),
        m_featureTable(features), m_range(range), m_is2d(is2d), m_maxMaterialsPerMesh(maxMaterialsPerMesh), m_maxVerticesPerMesh(maxVerticesPerMesh) { }

    DGNPLATFORM_EXPORT MeshBuilderR GetMeshBuilder(MeshBuilderKeyCR key, uint32_t numVertices);
    DGNPLATFORM_EXPORT MeshBuilderListPtr FindList(MeshBuilderKeyCR key) const;

    DRange3dCR GetRange() const { return m_range; }
    FeatureTableP GetFeatureTable() const { return m_featureTable; }
    double GetVertexTolerance() const { return m_vertexTolerance; }
    double GetFacetAreaTolerance() const { return m_facetAreaTolerance; }
    bool Is2d() const { return m_is2d; }
    uint8_t GetMaxMaterialsPerMesh() const { return m_maxMaterialsPerMesh; }
    uint32_t GetMaxVerticesPerMesh() const { return m_maxVerticesPerMesh; }

    size_t size() const { return m_set.size(); }

    void SetFeatureTable(FeatureTableR table) { m_featureTable = &table; }
    DGNPLATFORM_EXPORT void GetMeshes(MeshListR meshes) const;
    void GetMeshes(bvector<MeshPtr>& meshes, SharedGeomCR) const;
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Polyface
{
    DisplayParamsCPtr   m_displayParams;
    PolyfaceHeaderPtr   m_polyface;
    double              m_decimationTolerance;
    // If the original geometry was a polyface, this is the chord tolerance to use for decimation. Otherwise it is zero.
    bool                m_displayEdges;
    bool                m_isPlanar;
    Image*              m_glyphImage;

    Polyface(DisplayParamsCR displayParams, PolyfaceHeaderR polyface, bool displayEdges=true, bool isPlanar=false, Image* glyphImage=nullptr, double decimationTolerance=0.0)
        : m_displayParams(&displayParams), m_polyface(&polyface), m_displayEdges(displayEdges), m_isPlanar(isPlanar), m_glyphImage(glyphImage), m_decimationTolerance(decimationTolerance) { }

    void Transform(TransformCR transform) { if (m_polyface.IsValid()) m_polyface->Transform(transform); }
    Polyface Clone(DisplayParamsCR params) const { return Polyface(params, *m_polyface->Clone(), m_displayEdges, m_isPlanar, m_glyphImage, m_decimationTolerance); }
    Polyface Clone() const { return Clone(*m_displayParams); }
    DisplayParamsCR GetDisplayParams() const { return *m_displayParams; }
    bool CanDecimate() const { return 0.0 < m_decimationTolerance; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Strokes
{
    struct  PointList
    {
        double              m_startDistance;
        bvector<DPoint3d>   m_points;
        bool                m_canDecimate;

        PointList() : m_startDistance(0.0) { }
        PointList(double startDistance, bool canDecimate) : m_startDistance(startDistance), m_canDecimate(canDecimate) { }
        PointList(bvector<DPoint3d>&& points, bool canDecimate) : m_startDistance(0.0), m_points(std::move(points)), m_canDecimate(canDecimate) { }
        PointList(PointList&& src) : m_startDistance(src.m_startDistance), m_points(std::move(src.m_points)), m_canDecimate(src.m_canDecimate) {}

        PointList& operator=(PointList&& src)
            {
            if (&src != this)
                {
                m_startDistance = src.m_startDistance;
                m_points = std::move(src.m_points);
                m_canDecimate = src.m_canDecimate;
                }

            return *this;
            }
    };

    typedef bvector<PointList> PointLists;

    static PointLists ClipToRange(PointLists&& input, DRange3dCR range);

    DisplayParamsCPtr   m_displayParams;
    PointLists          m_strokes;
    double              m_decimationTolerance;
    bool                m_disjoint;
    bool                m_isPlanar;

    Strokes(DisplayParamsCR displayParams, PointLists&& strokes, bool disjoint, bool isPlanar, double decimationTolerance)
        : m_displayParams(&displayParams), m_strokes(std::move(strokes)), m_disjoint(disjoint), m_isPlanar(isPlanar), m_decimationTolerance(decimationTolerance) { }
    Strokes(DisplayParamsCR displayParams, bool disjoint, bool isPlanar, double decimationTolerance)
        : m_displayParams(&displayParams), m_disjoint(disjoint), m_isPlanar(isPlanar), m_decimationTolerance(decimationTolerance) { }
    Strokes(Strokes&& src)
        : m_displayParams(src.m_displayParams), m_strokes(std::move(src.m_strokes)), m_disjoint(src.m_disjoint), m_isPlanar(src.m_isPlanar), m_decimationTolerance(src.m_decimationTolerance) { }
    Strokes& operator=(Strokes&& src)
        {
        if (this != &src)
            {
            m_displayParams = src.m_displayParams;
            m_strokes = std::move(src.m_strokes);
            m_disjoint = src.m_disjoint;
            m_isPlanar = src.m_isPlanar;
            m_decimationTolerance = src.m_decimationTolerance;
            }

        return *this;
        }

    void Transform(TransformCR transform);
    DisplayParamsCR GetDisplayParams() const { return *m_displayParams; }
    uint32_t ComputePointCount() const;
    bool CanDecimate() const { return !m_disjoint && m_decimationTolerance > 0.0; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Geometry : RefCountedBase
{
private:
    DisplayParamsCPtr       m_params;
    Transform               m_transform;
    DRange3d                m_tileRange;
    DgnElementId            m_entityId;
    bool                    m_isCurved;
    bool                    m_hasTexture;
protected:
    ClipVectorPtr           m_clip;

    Geometry(TransformCR tf, DRange3dCR tileRange, DgnElementId entityId, DisplayParamsCR params, bool isCurved, DgnDbR db);

    virtual PolyfaceList _GetPolyfaces(double chordTolerance, NormalMode, ViewContextR) = 0;
    virtual StrokesList _GetStrokes(double chordTolerance, ViewContextR) { return StrokesList(); }
    virtual bool _DoVertexCluster() const { return true; }
    virtual SharedGeomCPtr _GetSharedGeom() const { return nullptr; }
    virtual void _SetInCache(bool inCache) { }
public:
    DisplayParamsCR GetDisplayParams() const { return *m_params; }
    TransformCR GetTransform() const { return m_transform; }
    DRange3dCR GetTileRange() const { return m_tileRange; }
    DgnElementId GetEntityId() const { return m_entityId; } //!< The ID of the element from which this geometry was produced
    Feature GetFeature() const { return m_params.IsValid() ? Feature(GetEntityId(), m_params->GetSubCategoryId(), m_params->GetClass()) : Feature(); }

    static IFacetOptionsPtr CreateFacetOptions(double chordTolerance, bool wantEdgeChains);

    static bool WantPolylineEdges(uint32_t lineWeight, bool is3d);

    bool IsCurved() const { return m_isCurved; }
    bool HasTexture() const { return m_hasTexture; }

    PolyfaceList GetPolyfaces(double chordTolerance, NormalMode normalMode, ViewContextR context);
    bool DoVertexCluster() const { return _DoVertexCluster(); }
    StrokesList GetStrokes (double chordTolerance, ViewContextR context);
    SharedGeomCPtr GetSharedGeom() const { return _GetSharedGeom(); }
    void SetInCache(bool inCache) { _SetInCache(inCache); }
    void SetClipVector(ClipVectorCP clip) { m_clip = nullptr != clip ? ClipVector::CreateCopy(*clip) : nullptr; }

    virtual bool IsInstanceable() const = 0;

    //! Create a Geometry for an IGeometry
    static GeometryPtr Create(IGeometryR geometry, TransformCR tf, DRange3dCR tileRange, DgnElementId entityId, DisplayParamsCR params, bool isCurved, DgnDbR db, bool disjoint);
    //! Create a Geometry for an IBRepEntity
    static GeometryPtr Create(IBRepEntityR solid, TransformCR tf, DRange3dCR tileRange, DgnElementId entityId, DisplayParamsCR params, DgnDbR db);
    //! Create a Geometry for text.
    static GeometryPtr Create(TextStringR textString, TransformCR transform, DRange3dCR range, DgnElementId entityId, DisplayParamsCR params, DgnDbR db, bool checkGlyphBoxes);
    //! Create a Geometry for a shared geometry instance.
    static GeometryPtr Create(SharedGeomR geom, TransformCR transform, DRange3dCR range, DgnElementId entityId, DisplayParamsCR params, DgnDbR db);
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   05/17
//=======================================================================================
struct GeometryList
{
    typedef bvector<GeometryPtr> List;
private:
    List    m_list;
    bool    m_complete = true;
    bool    m_curved = false;
public:
    GeometryList() = default;
    explicit GeometryList(GeometryR geom) : m_list(1)
        {
        m_list[0] = &geom;
        m_curved = geom.IsCurved();
        }

    bool IsComplete() const { return m_complete; }
    bool ContainsCurves() const { return m_curved; }
    void MarkIncomplete() { m_complete = false; }
    void MarkCurved() { m_curved = true; }

    typedef List::const_iterator const_iterator;

    const_iterator begin() const { return m_list.begin(); }
    const_iterator end() const { return m_list.end(); }
    bool empty() const { return m_list.empty(); }
    size_t size() const { return m_list.size(); }

    void push_back(GeometryR geom) { m_list.push_back(&geom); m_curved = m_curved || geom.IsCurved(); }
    void append(GeometryListR src) { m_list.insert(m_list.end(), src.m_list.begin(), src.m_list.end()); m_curved = m_curved || src.ContainsCurves(); }
    void resize(size_t newSize) { m_list.resize(newSize); }
    void clear() { m_list.clear(); }

    QPoint3d::Params ComputeQuantizationParams() const { return QPoint3d::Params(ComputeRange()); }
    DRange3d ComputeRange() const;

    GeometryList Slice(size_t startIndex, size_t endIndex) const;
};

//=======================================================================================
//! Represents shareable geometry, e.g. from a DgnGeometryPart or a SolidPrimitive.
//! Multiple instances can all refer to the single representation of the shared geometry,
//! with per-instance transform, element ID, and optional symbology overrides.
// @bsistruct                                                   Paul.Connelly   03/19
//=======================================================================================
struct SharedGeom : RefCountedBase
{
public:
    struct Instance
    {
    private:
        Transform           m_transform;
        DisplayParamsCPtr   m_displayParams;
        DgnElementId        m_elementId;
    public:
        Instance() { } // purely for compatibility with collection classes.
        Instance(TransformCR tf, DisplayParamsCR displayParams, DgnElementId elemId) : m_transform(tf), m_displayParams(&displayParams), m_elementId(elemId) { }

        DisplayParamsCR GetDisplayParams() const { return *m_displayParams; }
        TransformCR GetTransform() const { return m_transform; }
        DgnElementId GetElementId() const { return m_elementId; }
        Feature GetFeature() const { return Feature(GetElementId(), GetDisplayParams().GetSubCategoryId(), GetDisplayParams().GetClass()); }
    };

    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(Instance);

    using InstanceList = bvector<Instance>;

    // IMPORTANT: These values MUST match OvrFlags in iModel.js frontend (RenderFlags.ts)!!!
    enum class SymbologyOverrides : uint8_t
    {
        None = 0,
        Rgb = 1 << 1,
        Alpha = 1 << 2,
        LineWidth = 1 << 3,
        LinePixels = 1 << 6,
    };
protected:
    GeometryList        m_geometries;
private:
    DPoint3d            m_translation;
    DRange3d            m_range;
    InstanceList        m_instances;
    SymbologyOverrides  m_symbology = SymbologyOverrides::None;
    bool                m_instanceable = true;

    uint32_t _GetExcessiveRefCountThreshold() const  override {return 100000;}

protected:
    virtual DisplayParamsCR GetInstanceDisplayParams(GeometryCP instance, DisplayParamsCR geomParams) const;

    SharedGeom(DRange3dCR range, GeometryList const& geometries, DPoint3dCR translation);
    SharedGeom(DRange3dCR range, GeometryR geom, DPoint3dCR translation);
public:
    GeometryList const& GetGeometries() const { return m_geometries; }
    DRange3d GetRange() const { return m_range; };
    SymbologyOverrides GetSymbologyOverrides() const { return m_symbology; }

    // Some DgnGeometryParts define their geometry in wacky coordinate systems hundreds of kilometers from the origin.
    // To fix precision issues in display system, we translate it to the origin.
    // This returns the translation back to the part's coordinate system.
    DPoint3dCR GetTranslation() const { return m_translation; }

    InstanceList const& GetInstances() const { return m_instances; }
    size_t GetInstanceCount() const { return GetInstances().size(); }
    void AddInstance(TransformCR tf, DisplayParamsCR dispParams, DgnElementId elemId);

    void SetInCache(bool inCache);

    bool IsCurved() const;
    bool IsComplete() const { return m_geometries.IsComplete(); }
    PolyfaceList GetPolyfaces(double chordTolerance, NormalMode, GeometryCP instance, ViewContextR);
    StrokesList GetStrokes(double chordTolerance, GeometryCP instance, ViewContextR);

    virtual DisplayParamsCR GetDisplayParams() const = 0;
    virtual double GetTolerance(double baseTolerance) const { return baseTolerance; }
    bool IsWorthInstancing() const;
    virtual bool IsInstanceable() const { return m_instanceable; }

    DisplayParamsCPtr CloneDisplayParamsForInstance(DisplayParamsCR baseParams, DisplayParamsCR instanceParams) const;
};

ENUM_IS_FLAGS(SharedGeom::SymbologyOverrides);

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct PartGeom : SharedGeom
{
    DEFINE_T_SUPER(SharedGeom);

    struct Key
    {
        DgnGeometryPartId   m_partId;
        double              m_tolerance = 0.0;
        DisplayParamsCPtr   m_displayParams;

        Key() { } // Chiefly for use in collection types...
        Key(DgnGeometryPartId partId, double tolerance, DisplayParamsCR displayParams)
            : m_partId(partId), m_tolerance(tolerance), m_displayParams(&displayParams) { }

        bool IsLessThan(Key const& rhs, bool ignoreTolerance) const;
    };

    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(Key);

private:
    Key                 m_key;
    bool                m_hasSymbologyChanges;

protected:
    PartGeom(KeyCR key, DRange3dCR range, GeometryList const& geometry, bool hasSymbologyChanges, DPoint3dCR translation);

    static double ComputeTolerance(GeometryCP instance, double tolerance);

    DisplayParamsCR GetInstanceDisplayParams(GeometryCP, DisplayParamsCR) const override;
public:
    static PartGeomPtr Create(KeyCR key, DRange3dCR range, GeometryList const& geometry, bool hasSymbologyChanges, DPoint3dCR translation) { return new PartGeom(key, range, geometry, hasSymbologyChanges, translation); }

    bool IsInstanceable() const override { return !m_hasSymbologyChanges && T_Super::IsInstanceable(); }
    DisplayParamsCR GetDisplayParams() const override { return *m_key.m_displayParams; }
    double GetTolerance(double baseTolerance) const override { return m_key.m_tolerance; }
    bool IgnoresTolerance() const { return IsComplete() && !IsCurved(); }

    KeyCR GetKey() const { return m_key; }
    bool HasSymbologyChanges() const { return m_hasSymbologyChanges; }

    static double ComputeTolerance(TransformCR transform, double baseTolerance);

    struct PtrComparator
    {
        using is_transparent = std::true_type;

        bool operator()(PartGeomPtr const& lhs, PartGeomPtr const& rhs) const { return lhs->GetKey().IsLessThan(rhs->GetKey(), lhs->IgnoresTolerance()); }
        bool operator()(PartGeomPtr const& lhs, KeyCR rhs) const { return lhs->GetKey().IsLessThan(rhs, lhs->IgnoresTolerance()); }
        bool operator()(KeyCR lhs, PartGeomPtr const& rhs) const { return lhs.IsLessThan(rhs->GetKey(), rhs->IgnoresTolerance()); }
    };
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   03/19
//=======================================================================================
struct SolidPrimitiveGeom : SharedGeom
{
    struct Key
    {
    private:
        static constexpr double GetCompareTolerance() { return 1.0E-5; }
    public:
        ISolidPrimitivePtr  m_primitive;
        DisplayParamsCPtr   m_displayParams;
        int64_t             m_range[6];

        Key() { } // Chiefly for use in collection types...
        Key(Key const& other) = default;
        Key(ISolidPrimitiveR primitive, DisplayParamsCR params, DRange3dCR range);

        // For ordered comparison - compares range only. For use in set/map.
        bool operator<(Key const& rhs) const;
        // For rhs with same range, determine if both have the same geometry and compatible display params. For use in multiset/multimap.
        bool IsEquivalent(Key const& rhs) const;
    };

    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(Key);

private:
    Key m_key;

    SolidPrimitiveGeom(KeyCR key, DRange3dCR range, GeometryR geom, DPoint3dCR translation) : SharedGeom(range, geom, translation), m_key(key) { }
public:
    DisplayParamsCR GetDisplayParams() const override { return *m_key.m_displayParams; }
    KeyCR GetKey() const { return m_key; }

    struct PtrComparator
    {
        using is_transparent = std::true_type;

        bool operator()(SolidPrimitiveGeomPtr const& lhs, SolidPrimitiveGeomPtr const& rhs) const { return lhs->m_key < rhs->m_key; }
        bool operator()(SolidPrimitiveGeomPtr const& lhs, KeyCR rhs) const { return lhs->m_key < rhs; }
        bool operator()(KeyCR lhs, SolidPrimitiveGeomPtr const& rhs) const { return lhs < rhs->m_key; }
    };

    static SolidPrimitiveGeomPtr Create(KeyCR key, DRange3dCR range, DgnDbR db);
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct GeometryCollection
{
private:
    MeshList            m_meshes;
    bool                m_isComplete = true;
    bool                m_curved = false;
public:
    MeshList& Meshes()              { return m_meshes; }
    MeshList const& Meshes() const  { return m_meshes; }
    bool IsEmpty() const            { return m_meshes.empty(); }
    bool IsComplete() const         { return m_isComplete; }
    bool ContainsCurves() const     { return m_curved; }
    void MarkIncomplete()           { m_isComplete = false; }
    void MarkCurved()               { m_curved = true; }
};

//=======================================================================================
//! Accumulates a list of Geometry objects from a set of high-level graphics primitives.
//! The various Add() methods take ownership of the input object, which may later be
//! modified.
// @bsistruct                                                   Paul.Connelly   01/17
//=======================================================================================
struct GeometryAccumulator
{
private:
    GeometryList                m_geometries;
    Transform                   m_transform;
    DgnElementId                m_elementId;
    mutable DisplayParamsCache  m_displayParamsCache;
    bool                        m_surfacesOnly;
    bool                        m_haveTransform;
    bool                        m_checkGlyphBoxes = false;
    bool                        m_addingCurved = false;
    DRange3d                    m_tileRange;

    bool AddGeometry(IGeometryR geom, bool isCurved, DisplayParamsCR displayParams, TransformCR transform, ClipVectorCP clip, bool disjoint);
    bool AddGeometry(IGeometryR geom, bool isCurved, DisplayParamsCR displayParams, TransformCR transform, ClipVectorCP clip, DRange3dCR range, bool disjoint);
public:
    GeometryAccumulator(DgnDbR db, System& system, TransformCR transform, DRange3dCR tileRange, bool surfacesOnly) : m_transform(transform), m_tileRange(tileRange), m_displayParamsCache(db, system), m_surfacesOnly(surfacesOnly), m_haveTransform(!transform.IsIdentity()) { }
    GeometryAccumulator(DgnDbR db, System& system, bool surfacesOnly=false) : m_transform(Transform::FromIdentity()), m_displayParamsCache(db, system), m_surfacesOnly(surfacesOnly), m_haveTransform(false), m_tileRange(DRange3d::NullRange()) { }

    void AddGeometry(GeometryR geom) { m_geometries.push_back(geom); }
    void SetGeometryList(GeometryList const& geometries) { m_geometries = geometries; }

    DGNPLATFORM_EXPORT bool Add(CurveVectorR curves, bool filled, DisplayParamsCR displayParams, TransformCR transform, ClipVectorCP clip, bool disjoint);
    DGNPLATFORM_EXPORT bool Add(ISolidPrimitiveR primitive, DisplayParamsCR displayParams, TransformCR transform);
    DGNPLATFORM_EXPORT bool Add(RefCountedMSBsplineSurface& surface, DisplayParamsCR displayParams, TransformCR transform);
    DGNPLATFORM_EXPORT bool Add(PolyfaceHeaderR polyface, bool filled, DisplayParamsCR displayParams, TransformCR transform);
    DGNPLATFORM_EXPORT bool Add(IBRepEntityR body, DisplayParamsCR displayParams, TransformCR transform);
    DGNPLATFORM_EXPORT bool Add(TextStringR textString, DisplayParamsCR displayParams, TransformCR transform);
    bool AddTextUnderline(TextStringR, DisplayParamsCR, TransformCR);

    bool IsEmpty() const { return m_geometries.empty(); }
    void Clear() { m_geometries.clear(); }
    GeometryList const& GetGeometries() const { return m_geometries; }
    GeometryList& GetGeometries() { return m_geometries; }

    DgnElementId GetElementId() const { return m_elementId; }
    void SetElementId(DgnElementId id) { m_elementId = id; }

    TransformCR GetTransform() const { return m_transform; }
    void SetTransform(TransformCR tf) { m_transform = tf; m_haveTransform = !m_transform.IsIdentity(); }

    DgnDbR GetDgnDb() const { return m_displayParamsCache.GetDgnDb(); }
    System& GetSystem() const { return m_displayParamsCache.GetSystem(); }
    DisplayParamsCacheR GetDisplayParamsCache() const { return m_displayParamsCache; }
    bool WantSurfacesOnly() const { return m_surfacesOnly; }

    //! Convert the geometry accumulated by this builder into a set of meshes.
    DGNPLATFORM_EXPORT MeshList ToMeshes(GeometryOptionsCR options, double tolerance, ViewContextR, FeatureTableP featureTable=nullptr) const;
    DGNPLATFORM_EXPORT MeshBuilderSet ToMeshBuilders(GeometryOptionsCR options, double tolerance, FeatureTableP featureTable, ViewContextR) const;

    //! Populate a list of Graphic objects from the accumulated Geometry objects.
    DGNPLATFORM_EXPORT void SaveToGraphicList(bvector<Render::GraphicPtr>& graphics, GeometryOptionsCR options, double tolerance, ViewContextR, bool isWorldCoords) const;

    //! Clear the geometry list and reinitialize for reuse. DisplayParamsCache contents are preserved.
    void ReInitialize(TransformCR transform=Transform::FromIdentity(), DgnElementId elemId=DgnElementId(), bool surfacesOnly=false)
        {
        Clear();
        SetTransform(transform);
        SetElementId(elemId);
        m_surfacesOnly = surfacesOnly;
        }

    //! If enabled, TextString range will be tested against chord tolerance to determine whether the text should be stroked or rendered as a simple box.
    //! By default, it is always stroked.
    void SetCheckGlyphBoxes(bool check) { m_checkGlyphBoxes = check; }
    void SetAddingCurved(bool curved) { m_addingCurved = curved; }
    bool IsAddingCurved() const { return m_addingCurved; }
};

//=======================================================================================
//! Implements GraphicBuilder to accumulate high-level graphics primitives into a list
//! of Render::Primitives::Geometry objects.
// @bsistruct                                                   Paul.Connelly   05/17
//=======================================================================================
struct GeometryListBuilder : GraphicBuilder
{
private:
    GeometryAccumulator m_accum;
    GraphicParams       m_graphicParams;
    GeometryParams      m_geometryParams;
    bool                m_geometryParamsValid = false;
    bool                m_isOpen = true;

    bool _IsOpen() const final override { return m_isOpen; }
    DGNPLATFORM_EXPORT Render::GraphicPtr _Finish() final override;
protected:
    GeometryListBuilder(System& system, CreateParams const& params, DgnElementId elemId=DgnElementId(), TransformCR accumulatorTf=Transform::FromIdentity())
        : GraphicBuilder(params), m_accum(params.GetDgnDb(), system) { m_accum.SetElementId(elemId); m_accum.SetTransform(accumulatorTf); }

    DGNPLATFORM_EXPORT void _ActivateGraphicParams(GraphicParamsCR, Render::GeometryParamsCP) override;
    DGNPLATFORM_EXPORT void _AddArc2d(DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth) override;
    DGNPLATFORM_EXPORT void _AddArc(DEllipse3dCR ellipse, bool isEllipse, bool filled) override;
    DGNPLATFORM_EXPORT void _AddLineString2d(int numPoints, DPoint2dCP points, double zDepth) override;
    DGNPLATFORM_EXPORT void _AddLineString(int numPoints, DPoint3dCP points) override;
    DGNPLATFORM_EXPORT void _AddPointString2d(int numPoints, DPoint2dCP points, double zDepth) override;
    DGNPLATFORM_EXPORT void _AddPointString(int numPoints, DPoint3dCP points) override;
    DGNPLATFORM_EXPORT void _AddPolyface(PolyfaceQueryCR, bool filled) override;
    DGNPLATFORM_EXPORT void _AddBody(IBRepEntityCR) override;
    DGNPLATFORM_EXPORT void _AddShape2d(int numPoints, DPoint2dCP points, bool filled, double zDepth) override;
    DGNPLATFORM_EXPORT void _AddShape(int numPoints, DPoint3dCP points, bool filled) override;
    DGNPLATFORM_EXPORT void _AddTextString2d(TextStringCR, double zDepth) override;
    DGNPLATFORM_EXPORT void _AddTextString(TextStringCR) override;
    DGNPLATFORM_EXPORT void _AddTriStrip(int numPoints, DPoint3dCP points, AsThickenedLine usageFlags) override;
    DGNPLATFORM_EXPORT void _AddTriStrip2d(int numPoints, DPoint2dCP points, AsThickenedLine usageFlags, double zDepth) override;
    DGNPLATFORM_EXPORT void _AddSolidPrimitive(ISolidPrimitiveCR primitive) override;
    DGNPLATFORM_EXPORT void _AddCurveVector(CurveVectorCR curves, bool isFilled) override;
    DGNPLATFORM_EXPORT void _AddCurveVector2d(CurveVectorCR curves, bool isFilled, double zDepth) override;
    DGNPLATFORM_EXPORT void _AddBSplineCurve(MSBsplineCurveCR, bool filled) override;
    DGNPLATFORM_EXPORT void _AddBSplineCurve2d(MSBsplineCurveCR, bool filled, double zDepth) override;
    DGNPLATFORM_EXPORT void _AddBSplineSurface(MSBsplineSurfaceCR) override;
    DGNPLATFORM_EXPORT void _AddDgnOle(DgnOleDraw*) override;

    DGNPLATFORM_EXPORT void _AddBSplineCurveR(RefCountedMSBsplineCurveR curve, bool filled) override;
    DGNPLATFORM_EXPORT void _AddBSplineCurve2dR(RefCountedMSBsplineCurveR curve, bool filled, double zDepth) override;
    DGNPLATFORM_EXPORT void _AddCurveVectorR(CurveVectorR curves, bool isFilled) override;
    DGNPLATFORM_EXPORT void _AddCurveVector2dR(CurveVectorR curves, bool isFilled, double zDepth) override;
    DGNPLATFORM_EXPORT void _AddSolidPrimitiveR(ISolidPrimitiveR primitive) override;
    DGNPLATFORM_EXPORT void _AddBSplineSurfaceR(RefCountedMSBsplineSurfaceR surface) override;
    DGNPLATFORM_EXPORT void _AddPolyfaceR(PolyfaceHeaderR meshData, bool filled = false) override;
    DGNPLATFORM_EXPORT void _AddBodyR(IBRepEntityR body) override;
    DGNPLATFORM_EXPORT void _AddTextStringR(TextStringR text) override;
    DGNPLATFORM_EXPORT void _AddTextString2dR(TextStringR text, double zDepth) override;

    virtual Render::GraphicPtr _FinishGraphic(GeometryAccumulatorR) = 0; //!< Invoked by _Finish() to obtain the finished Graphic.
    virtual void _Reset() { } //!< Invoked by ReInitialize() to reset any state before this builder is reused.

    void Add(PolyfaceHeaderR mesh, bool filled) { m_accum.Add(mesh, filled, GetMeshDisplayParams(filled), GetLocalToWorldTransform()); }
    void AddCurveVector(CurveVectorR curves, bool isFilled, bool isDisjoint);
    void SetCheckGlyphBoxes(bool check) { m_accum.SetCheckGlyphBoxes(check); }
    void SetElementId(DgnElementId id) { m_accum.SetElementId(id); }
public:
    GraphicParamsCR GetGraphicParams() const { return m_graphicParams; }
    GeometryParamsCP GetGeometryParams() const { return m_geometryParamsValid ? &m_geometryParams : nullptr; }
    DgnElementId GetElementId() const { return m_accum.GetElementId(); }

    DisplayParamsCacheR GetDisplayParamsCache() const { return m_accum.GetDisplayParamsCache(); }
    DisplayParamsCR GetDisplayParams(DisplayParams::Type type, bool filled) const { return m_accum.GetDisplayParamsCache().Get(type, GetGraphicParams(), GetGeometryParams(), filled); }
    DisplayParamsCR GetMeshDisplayParams(bool filled) const { return GetDisplayParams(DisplayParams::Type::Mesh, filled); }
    DisplayParamsCR GetLinearDisplayParams() const { return GetDisplayParams(DisplayParams::Type::Linear, false); }
    DisplayParamsCR GetTextDisplayParams() const { return GetDisplayParams(DisplayParams::Type::Text, false); }

    System& GetSystem() const { return m_accum.GetSystem(); }

    void SetAddingCurved(bool curved) { m_accum.SetAddingCurved(curved); }
    bool IsAddingCurved() const { return m_accum.IsAddingCurved(); }

    void Add(GeometryR geom) { m_accum.AddGeometry(geom); }

    //! Reset this builder for reuse.
    DGNPLATFORM_EXPORT void ReInitialize(TransformCR localToWorld, TransformCR accumulatorTransform=Transform::FromIdentity(), DgnElementId elemId=DgnElementId());

    static double DepthFromDisplayPriority(int32_t priority);
};

//=======================================================================================
//! General-purpose specialization of GeometryListBuilder which produces Graphic objects
//! using a supplied Render::System.
// @bsistruct                                                   Paul.Connelly   03/17
//=======================================================================================
struct PrimitiveBuilder : GeometryListBuilder
{
protected:
    bvector<GraphicPtr> m_primitives;

    DGNPLATFORM_EXPORT void _AddSubGraphic(Render::Graphic&, TransformCR, Render::GraphicParamsCR, ClipVectorCP clip) override;
    DGNPLATFORM_EXPORT Render::GraphicBuilderPtr _CreateSubGraphic(TransformCR, ClipVectorCP clip) const override;
    DGNPLATFORM_EXPORT Render::GraphicPtr _FinishGraphic(GeometryAccumulatorR) override;
    void _Reset() override { m_primitives.clear(); }

    DGNPLATFORM_EXPORT double ComputeTolerance(GeometryAccumulatorR) const;
public:
    PrimitiveBuilder(System& system, Render::GraphicBuilder::CreateParams const& params, DgnElementId elemId=DgnElementId()) : GeometryListBuilder(system, params, elemId) { }
};

END_BENTLEY_RENDER_PRIMITIVES_NAMESPACE


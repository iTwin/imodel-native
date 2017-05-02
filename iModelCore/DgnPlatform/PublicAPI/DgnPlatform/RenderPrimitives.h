/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/RenderPrimitives.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/Render.h>
#include <DgnPlatform/DgnTexture.h>
#include <DgnPlatform/DgnMaterial.h>

#define BEGIN_BENTLEY_RENDER_PRIMITIVES_NAMESPACE BEGIN_BENTLEY_RENDER_NAMESPACE namespace Primitives {
#define END_BENTLEY_RENDER_PRIMITIVES_NAMESPACE } END_BENTLEY_RENDER_NAMESPACE
#define USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES using namespace BentleyApi::Dgn::Render::Primitives;

BEGIN_BENTLEY_RENDER_PRIMITIVES_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(DisplayParams);
DEFINE_POINTER_SUFFIX_TYPEDEFS(DisplayParamsCache);
DEFINE_POINTER_SUFFIX_TYPEDEFS(MeshInstance);
DEFINE_POINTER_SUFFIX_TYPEDEFS(MeshPart);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Triangle);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Polyline);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Mesh);
DEFINE_POINTER_SUFFIX_TYPEDEFS(MeshMergeKey);
DEFINE_POINTER_SUFFIX_TYPEDEFS(MeshBuilder);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Geometry);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Polyface);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Strokes);
DEFINE_POINTER_SUFFIX_TYPEDEFS(GeometryCollection);
DEFINE_POINTER_SUFFIX_TYPEDEFS(VertexKey);
DEFINE_POINTER_SUFFIX_TYPEDEFS(TriangleKey);
DEFINE_POINTER_SUFFIX_TYPEDEFS(GeomPart);
DEFINE_POINTER_SUFFIX_TYPEDEFS(GeometryOptions);
DEFINE_POINTER_SUFFIX_TYPEDEFS(GeometryListBuilder);
DEFINE_POINTER_SUFFIX_TYPEDEFS(ColorTable);
DEFINE_POINTER_SUFFIX_TYPEDEFS(PrimitiveBuilder);

DEFINE_REF_COUNTED_PTR(DisplayParams);
DEFINE_REF_COUNTED_PTR(MeshPart);
DEFINE_REF_COUNTED_PTR(Mesh);
DEFINE_REF_COUNTED_PTR(MeshBuilder);
DEFINE_REF_COUNTED_PTR(Geometry);
DEFINE_REF_COUNTED_PTR(GeomPart);

typedef bvector<MeshInstance>       MeshInstanceList;
typedef bvector<MeshPartPtr>        MeshPartList;
typedef bvector<GeometryPtr>        GeometryList;
typedef bvector<Triangle>           TriangleList;
typedef bvector<Polyline>           PolylineList;
typedef bvector<Polyface>           PolyfaceList;
typedef bvector<Strokes>            StrokesList;
typedef bmap<double, PolyfaceList>  PolyfaceMap;

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
enum class NormalMode
{
    Never,              //!< Never generate normals
    Always,             //!< Always generate normals
    CurvedSurfacesOnly, //!< Generate normals only for curved surfaces
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct GeometryOptions
{
    enum class SurfacesOnly { Yes, No };
    enum class TwoSidedTriangles { Yes, No };

    NormalMode          m_normalMode;
    SurfacesOnly        m_surfaces;
    TwoSidedTriangles   m_twoSidedTriangles;

    explicit GeometryOptions(NormalMode normals=NormalMode::Always, SurfacesOnly surfaces=SurfacesOnly::No, TwoSidedTriangles twoSidedTriangles=TwoSidedTriangles::No)
        : m_normalMode(normals), m_surfaces(surfaces), m_twoSidedTriangles(twoSidedTriangles) { }

    bool WantSurfacesOnly() const { return SurfacesOnly::Yes == m_surfaces; }
    bool WantTwoSidedTriangles() const { return TwoSidedTriangles::Yes == m_twoSidedTriangles; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct DisplayParams : RefCountedBase
{
    friend struct DisplayParamsCache;
private:
    enum class IsTextured { Yes, No, Maybe };

    Render::GraphicParams           m_graphicParams;
    Render::GeometryParams          m_geometryParams;
    mutable Render::TexturePtr      m_texture;
    mutable IsTextured              m_isTextured;
    bool                            m_ignoreLighting;
    bool                            m_geometryParamsValid;

    DGNPLATFORM_EXPORT DisplayParams(Render::GraphicParamsCR graphicParams, Render::GeometryParamsCP geometryParams, bool ignoreLighting);

    uint32_t _GetExcessiveRefCountThreshold() const override { return 0x7fffffff; }

    static DisplayParamsCPtr Create() { return Create(Render::GraphicParams(), nullptr); }
    static DisplayParamsCPtr Create(ColorDef fillColor, Render::GeometryParamsCR geometryParams, bool ignoreLighting=false)
        { Render::GraphicParams gfParams; gfParams.SetFillColor(fillColor); return Create(gfParams, geometryParams, ignoreLighting); }
    static DisplayParamsCPtr Create(Render::GraphicParamsCR graphicParams, Render::GeometryParamsCR geometryParams, bool ignoreLighting=false)
        { return Create(graphicParams, &geometryParams, ignoreLighting); }

    DisplayParamsCPtr Clone() const;

    DgnTextureCPtr QueryTexture(DgnDbR db) const;
public:
    static DisplayParamsCPtr Create(Render::GraphicParamsCR graphicParams, Render::GeometryParamsCP geometryParams, bool ignoreLighting=false)
        { return new DisplayParams(graphicParams, geometryParams, ignoreLighting); }

    Render::GraphicParamsCR GetGraphicParams() const { return m_graphicParams; }
    Render::GeometryParamsCP GetGeometryParams() const { return HasGeometryParams() ? &m_geometryParams : nullptr; }
    bool GetIgnoreLighting() const { return m_ignoreLighting; }
    bool HasGeometryParams() const { return m_geometryParamsValid; }
    bool HasTransparency() const { return 0 != GetFillColorDef().GetAlpha(); }

    DgnCategoryId GetCategoryId() const { return HasGeometryParams() ? m_geometryParams.GetCategoryId() : DgnCategoryId(); }
    DgnSubCategoryId GetSubCategoryId() const { return HasGeometryParams() ? GetGeometryParams()->GetSubCategoryId() : DgnSubCategoryId(); }
    DgnMaterialId GetMaterialId() const { return HasGeometryParams() ? GetGeometryParams()->GetMaterialId() : DgnMaterialId(); }
    ColorDef GetFillColorDef() const { return GetGraphicParams().GetFillColor(); }
    uint32_t GetFillColor() const { return GetFillColorDef().GetValue(); }
    GradientSymbCP GetGradient() const { return GetGraphicParams().GetGradientSymb(); }
    uint32_t GetRasterWidth() const { return GetGraphicParams().GetWidth(); }
    uint32_t GetLinePixels() const { return GetGraphicParams().GetLinePixels(); }
    Render::DgnGeometryClass GetClass() const { return HasGeometryParams() ? GetGeometryParams()->GetGeometryClass() : Render::DgnGeometryClass::Primary; }

    DGNPLATFORM_EXPORT bool HasTexture(DgnDbR db) const;
    DGNPLATFORM_EXPORT Render::TextureP ResolveTexture(DgnDbR db, Render::System const& system) const;

    enum class ComparePurpose
    {
        Merge,  // ignores category, subcategory, class, and considers fill colors equivalent if both have or both lack transparency
        Strict  // compares all members
    };

    DGNPLATFORM_EXPORT bool IsLessThan(DisplayParamsCR rhs, ComparePurpose purpose=ComparePurpose::Strict) const;
    DGNPLATFORM_EXPORT bool IsEqualTo(DisplayParamsCR rhs, ComparePurpose purpose=ComparePurpose::Strict) const;
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   03/17
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

    Set     m_set;

    DisplayParamsCR Get(DisplayParamsCR params);
public:
    DisplayParamsCR GetDefault()
        {
        return Get(Render::GraphicParams(), nullptr);
        }
    DisplayParamsCR Get(ColorDef fill, Render::GeometryParamsCR geomParams, bool ignoreLighting=false)
        {
        Render::GraphicParams graphicParams;
        graphicParams.SetFillColor(fill);
        return Get(graphicParams, geomParams, ignoreLighting);
        }
    DisplayParamsCR Get(Render::GraphicParamsCR graphicParams, Render::GeometryParamsCR geomParams, bool ignoreLighting=false)
        {
        return Get(graphicParams, &geomParams, ignoreLighting);
        }
    DisplayParamsCR Get(Render::GraphicParamsCR graphicParams, Render::GeometryParamsCP geomParams, bool ignoreLighting=false)
        {
        return Get(DisplayParams(graphicParams, geomParams, ignoreLighting));
        }
};

//=======================================================================================
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

    void ToColorIndex(ColorIndex& index, bvector<uint32_t>& colors, bvector<uint16_t> const& indices) const;
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct MeshInstance
{
private:
    Feature     m_feature;
    Transform   m_transform;
public:
    MeshInstance(FeatureCR feature, TransformCR transform) : m_feature(feature), m_transform(transform) { }

    FeatureCR GetFeature() const { return m_feature; }
    TransformCR GetTransform() const { return m_transform; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   03/17
//=======================================================================================
struct MeshList : bvector<MeshPtr>
{
    FeatureTable    m_features;

    explicit MeshList(uint32_t maxFeatures=2048*1024) : m_features(maxFeatures) { }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct MeshPart : RefCountedBase
{
private:
    MeshList            m_meshes;
    MeshInstanceList    m_instances;

    MeshPart(MeshList&& meshes) : m_meshes(std::move(meshes)) { }

    uint32_t _GetExcessiveRefCountThreshold() const override { return 100000; }
public:
    static MeshPartPtr Create(MeshList&& meshes) { return new MeshPart(std::move(meshes)); }

    MeshList const& Meshes() const { return m_meshes; }
    MeshList& Meshes() { return m_meshes; }
    MeshInstanceList const& Instances() const { return m_instances; }
    void AddInstance(MeshInstanceCR instance) { m_instances.push_back(instance); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Triangle
{
    uint32_t    m_indices[3];
    bool        m_singleSided;

    explicit Triangle(bool singleSided=true) : m_singleSided(singleSided) { SetIndices(0, 0, 0); }
    Triangle(uint32_t indices[3], bool singleSided) : m_singleSided(singleSided) { SetIndices(indices); }
    Triangle(uint32_t a, uint32_t b, uint32_t c, bool singleSided) : m_singleSided(singleSided) { SetIndices(a, b, c); }

    void SetIndices(uint32_t indices[3]) { SetIndices(indices[0], indices[1], indices[2]); }
    void SetIndices(uint32_t a, uint32_t b, uint32_t c) { m_indices[0] = a; m_indices[1] = b; m_indices[2] = c; }

    bool IsDegenerate() const
        {
        return m_indices[0] == m_indices[1] || m_indices[0] == m_indices[2] || m_indices[1] == m_indices[2];
        }                                   
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Polyline
{
private:
    bvector<uint32_t>   m_indices;
public:
    bvector<uint32_t> const& GetIndices() const { return m_indices; }
    bvector<uint32_t>& GetIndices() { return m_indices; }
    void AddIndex(uint32_t index)  { if (m_indices.empty() || m_indices.back() != index) m_indices.push_back(index); }
    void Clear() { m_indices.clear(); }
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
        void ToFeatureIndex(FeatureIndex& index) const;
    };

    DisplayParamsCPtr       m_displayParams;
    TriangleList            m_triangles;
    PolylineList            m_polylines;
    bvector<FPoint3d>       m_points;
    bvector<FPoint3d>       m_normals;
    bvector<FPoint2d>       m_uvParams;
    ColorTable              m_colorTable;
    bvector<uint16_t>       m_colors;
    Features                m_features;
    PrimitiveType           m_type;

    Mesh(DisplayParamsCR params, FeatureTableP featureTable, PrimitiveType type) : m_displayParams(&params), m_features(featureTable), m_type(type) { }

    template<typename T> T const* GetMember(bvector<T> const& from, uint32_t at) const { return at < from.size() ? &from[at] : nullptr; }

    DPoint3d GetDPoint3d(bvector<FPoint3d> const& from, uint32_t index) const;

    friend struct MeshBuilder;
    void SetDisplayParams(DisplayParamsCR params) { m_displayParams = &params; }
public:
    static MeshPtr Create(DisplayParamsCR params, FeatureTableP featureTable, PrimitiveType type) { return new Mesh(params, featureTable, type); }

    DGNPLATFORM_EXPORT DRange3d GetTriangleRange(TriangleCR triangle) const;
    DGNPLATFORM_EXPORT DVec3d GetTriangleNormal(TriangleCR triangle) const;
    DGNPLATFORM_EXPORT bool HasNonPlanarNormals() const;

    DisplayParamsCR GetDisplayParams() const { return *m_displayParams; } //!< The mesh symbology
    DisplayParamsCPtr GetDisplayParamsPtr() const { return m_displayParams; } //!< The mesh symbology
    TriangleList const& Triangles() const { return m_triangles; } //!< Triangles defined as a set of 3 indices into the vertex attribute arrays.
    PolylineList const& Polylines() const { return m_polylines; } //!< Polylines defined as a set of indices into the vertex attribute arrays.
    bvector<FPoint3d> const& Points() const { return m_points; } //!< Position vertex attribute array
    bvector<FPoint3d> const& Normals() const { return m_normals; } //!< Normal vertex attribute array
    bvector<FPoint2d> const& Params() const { return m_uvParams; } //!< UV params vertex attribute array
    bvector<uint16_t> const& Colors() const { return m_colors; } //!< Vertex attribute array specifying an index into the color table
    ColorTableCR GetColorTable() const { return m_colorTable; }
    void ToFeatureIndex(FeatureIndex& index) const { m_features.ToFeatureIndex(index); }

    bool IsEmpty() const { return m_triangles.empty() && m_polylines.empty(); }
    PrimitiveType GetType() const { return m_type; }

    DGNPLATFORM_EXPORT DRange3d GetRange() const;
    DGNPLATFORM_EXPORT DRange3d GetUVRange() const;

    void AddTriangle(TriangleCR triangle) { BeAssert(PrimitiveType::Mesh == GetType()); m_triangles.push_back(triangle); }
    void AddPolyline(PolylineCR polyline) { BeAssert(PrimitiveType::Polyline == GetType() || PrimitiveType::Point == GetType()); m_polylines.push_back(polyline); }
    uint32_t AddVertex(DPoint3dCR point, DVec3dCP normal, DPoint2dCP param, uint32_t fillColor, FeatureCR feature);
};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     06/2016
+===============+===============+===============+===============+===============+======*/
struct MeshMergeKey
{
    DisplayParamsCP     m_params;                                                                                                                                                     
    bool                m_hasNormals;
    Mesh::PrimitiveType m_primitiveType;

    MeshMergeKey() : m_params(nullptr), m_hasNormals(false), m_primitiveType(Mesh::PrimitiveType::Mesh) { }
    MeshMergeKey(DisplayParamsCR params, bool hasNormals, Mesh::PrimitiveType type) : m_params(&params), m_hasNormals(hasNormals), m_primitiveType(type) { }
    MeshMergeKey(MeshCR mesh) : MeshMergeKey(mesh.GetDisplayParams(),  !mesh.Normals().empty(), mesh.GetType()) { }

    bool operator<(MeshMergeKey const& rhs) const
        {
        BeAssert(nullptr != m_params && nullptr != rhs.m_params);
        if(m_hasNormals != rhs.m_hasNormals)
            return !m_hasNormals;

        if (m_primitiveType != rhs.m_primitiveType)
            return m_primitiveType < rhs.m_primitiveType;

        return m_params->IsLessThan(*rhs.m_params, DisplayParams::ComparePurpose::Merge);
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct VertexKey
{
    struct Flags
    {
        union
        {
            uint8_t byteVal;
            struct
            {
                uint32_t    normalValid: 1;
                uint32_t    paramValid: 1;
                uint32_t    pointXLessThanY: 1;
                uint32_t    normalXLessThanY: 1;
                uint32_t    paramXLessThanY: 1;
            };
        };

        Flags() : byteVal(0) { }
        Flags(DPoint3dCR point, DVec3dCP normal, DPoint2dCP param);
    };

    DPoint3d        m_point;
    DVec3d          m_normal;
    DPoint2d        m_param;
    Feature         m_feature;
    uint32_t        m_fillColor = 0;
    Flags           m_flags;

    VertexKey() { }
    VertexKey(DPoint3dCR point, DVec3dCP normal, DPoint2dCP param, FeatureCR feature, uint32_t fillColor) : m_point(point), m_feature(feature), m_fillColor(fillColor), m_flags(point, normal, param)
        {
        if(m_flags.normalValid) m_normal = *normal;
        if(m_flags.paramValid) m_param = *param;
        }

    DVec3dCP GetNormal() const { return m_flags.normalValid ? &m_normal : nullptr; }
    DPoint2dCP GetParam() const { return m_flags.paramValid ? &m_param : nullptr; }

    //=======================================================================================
    // @bsistruct                                                   Paul.Connelly   12/16
    //=======================================================================================
    struct Comparator
    {
        double  m_tolerance;

        explicit Comparator(double tolerance) : m_tolerance(tolerance) { }
        bool operator()(VertexKey const& lhs, VertexKey const& rhs) const;
    };
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

typedef bmap<VertexKey, uint32_t, VertexKey::Comparator> VertexMap;
typedef bset<TriangleKey> TriangleSet;

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct MeshBuilder : RefCountedBase
{
private:
    MeshPtr             m_mesh;
    VertexMap           m_clusteredVertexMap;
    VertexMap           m_unclusteredVertexMap;
    TriangleSet         m_triangleSet;
    double              m_tolerance;
    double              m_areaTolerance;
    DgnMaterialCPtr     m_materialEl;
    RenderingAssetCP    m_material = nullptr;

    MeshBuilder(DisplayParamsCR params, double tolerance, double areaTolerance, FeatureTableP featureTable, Mesh::PrimitiveType type)
        : m_mesh(Mesh::Create(params, featureTable, type)), m_unclusteredVertexMap(VertexKey::Comparator(1.0E-4)), m_clusteredVertexMap(VertexKey::Comparator(tolerance)), 
          m_tolerance(tolerance), m_areaTolerance(areaTolerance) { }

    bool GetMaterial(DgnMaterialId materailId, DgnDbR db);
    uint32_t AddVertex(VertexMap& vertices, VertexKeyCR vertex);
public:
    static MeshBuilderPtr Create(DisplayParamsCR params, double tolerance, double areaTolerance, FeatureTableP featureTable, Mesh::PrimitiveType type)
        { return new MeshBuilder(params, tolerance, areaTolerance, featureTable, type); }

    DGNPLATFORM_EXPORT void AddTriangle(PolyfaceVisitorR visitor, DgnMaterialId materialId, DgnDbR dgnDb, FeatureCR feature, bool doVertexClustering, bool duplicateTwoSidedTriangles, bool includeParams, uint32_t fillColor);
    DGNPLATFORM_EXPORT void AddPolyline(bvector<DPoint3d>const& polyline, FeatureCR feature, bool doVertexClustering, uint32_t fillColor);
    DGNPLATFORM_EXPORT void AddPolyface(PolyfaceQueryCR polyface, DgnMaterialId materialId, DgnDbR dgnDb, FeatureCR feature, bool duplicateTwoSidedTriangles, bool includeParams, uint32_t fillColor);

    void AddMesh(TriangleCR triangle);
    void AddTriangle(TriangleCR triangle);
    uint32_t AddClusteredVertex(VertexKey const& vertex) { return AddVertex(m_clusteredVertexMap, vertex); }
    uint32_t AddVertex(VertexKey const& vertex) { return AddVertex(m_unclusteredVertexMap, vertex); }

    MeshP GetMesh() { return m_mesh.get(); } //!< The mesh under construction
    double GetTolerance() const { return m_tolerance; }

    void SetDisplayParams(DisplayParamsCR params) { m_mesh->SetDisplayParams(params); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Polyface
{
    DisplayParamsCPtr   m_displayParams;
    PolyfaceHeaderPtr   m_polyface;

    Polyface(DisplayParamsCR displayParams, PolyfaceHeaderR polyface) : m_displayParams(&displayParams), m_polyface(&polyface) { }

    void Transform(TransformCR transform) { if (m_polyface.IsValid()) m_polyface->Transform(transform); }
    Polyface Clone() const { return Polyface(*m_displayParams, *m_polyface->Clone()); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Strokes
{
    typedef bvector<bvector<DPoint3d>> PointLists;

    DisplayParamsCPtr   m_displayParams;
    PointLists          m_strokes;
    bool                m_disjoint;

    Strokes(DisplayParamsCR displayParams, PointLists&& strokes, bool disjoint) : m_displayParams(&displayParams), m_strokes(std::move(strokes)), m_disjoint(disjoint) { }

    void Transform(TransformCR transform);
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
    mutable size_t          m_facetCount;
    bool                    m_isCurved;
    bool                    m_hasTexture;
protected:
    Geometry(TransformCR tf, DRange3dCR tileRange, DgnElementId entityId, DisplayParamsCR params, bool isCurved, DgnDbR db);

    virtual PolyfaceList _GetPolyfaces(IFacetOptionsR facetOptions) = 0;
    virtual StrokesList _GetStrokes (IFacetOptionsR facetOptions) { return StrokesList(); }
    virtual bool _DoDecimate() const { return false; }
    virtual bool _DoVertexCluster() const { return true; }
    virtual size_t _GetFacetCount(FacetCounter& counter) const = 0;
    virtual GeomPartCPtr _GetPart() const { return nullptr; }
    virtual void _SetInCache(bool inCache) { }

    void SetFacetCount(size_t numFacets);
public:
    DisplayParamsCR GetDisplayParams() const { return *m_params; }
    DisplayParamsCPtr GetDisplayParamsPtr() const { return m_params; }
    TransformCR GetTransform() const { return m_transform; }
    DRange3dCR GetTileRange() const { return m_tileRange; }
    DgnElementId GetEntityId() const { return m_entityId; } //!< The ID of the element from which this geometry was produced
    size_t GetFacetCount(IFacetOptionsR options) const;
    size_t GetFacetCount(FacetCounter& counter) const { return _GetFacetCount(counter); }
    
    Feature GetFeature() const { return m_params.IsValid() ? Feature(GetEntityId(), m_params->GetSubCategoryId(), m_params->GetClass()) : Feature(); }

    static IFacetOptionsPtr CreateFacetOptions(double chordTolerance);
    IFacetOptionsPtr CreateFacetOptions(double chordTolerance, NormalMode normalMode) const;

    bool IsCurved() const { return m_isCurved; }
    bool HasTexture() const { return m_hasTexture; }

    PolyfaceList GetPolyfaces(IFacetOptionsR facetOptions) { return _GetPolyfaces(facetOptions); }
    PolyfaceList GetPolyfaces(double chordTolerance, NormalMode normalMode);
    bool DoDecimate() const { return _DoDecimate(); }
    bool DoVertexCluster() const { return _DoVertexCluster(); }
    StrokesList GetStrokes (IFacetOptionsR facetOptions) { return _GetStrokes(facetOptions); }
    GeomPartCPtr GetPart() const { return _GetPart(); }
    void SetInCache(bool inCache) { _SetInCache(inCache); }

    //! Create a Geometry for an IGeometry
    static GeometryPtr Create(IGeometryR geometry, TransformCR tf, DRange3dCR tileRange, DgnElementId entityId, DisplayParamsCR params, bool isCurved, DgnDbR db);
    //! Create a Geometry for an IBRepEntity
    static GeometryPtr Create(IBRepEntityR solid, TransformCR tf, DRange3dCR tileRange, DgnElementId entityId, DisplayParamsCR params, DgnDbR db);
    //! Create a Geometry for text.
    static GeometryPtr Create(TextStringR textString, TransformCR transform, DRange3dCR range, DgnElementId entityId, DisplayParamsCR params, DgnDbR db);
    //! Create a Geometry for a part instance.
    static GeometryPtr Create(GeomPartR part, TransformCR transform, DRange3dCR range, DgnElementId entityId, DisplayParamsCR params, DgnDbR db);
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct GeomPart : RefCountedBase
{
private:
    DRange3d                m_range;
    GeometryList            m_geometries;
    mutable size_t          m_facetCount;

    uint32_t _GetExcessiveRefCountThreshold() const  override {return 100000;}

protected:
    GeomPart(DRange3dCR range, GeometryList const& geometry);

public:
    static GeomPartPtr Create(DRange3dCR range, GeometryList const& geometry) { return new GeomPart(range, geometry); }
    PolyfaceList GetPolyfaces(IFacetOptionsR facetOptions, GeometryCR instance);
    PolyfaceList GetPolyfaces(IFacetOptionsR facetOptions, GeometryCP instance);
    StrokesList GetStrokes(IFacetOptionsR facetOptions, GeometryCR instance);
    StrokesList GetStrokes(IFacetOptionsR facetOptions, GeometryCP instance);
    size_t GetFacetCount(FacetCounter& counter, GeometryCR instance) const;
    bool IsCurved() const;
    GeometryList const& GetGeometries() const { return m_geometries; }
    DRange3d GetRange() const { return m_range; };
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct GeometryCollection
{
private:
    MeshList            m_meshes;
    MeshPartList        m_parts;
public:
    MeshList& Meshes()              { return m_meshes; }
    MeshPartList& Parts()           { return m_parts; }
    bool IsEmpty() const            { return m_meshes.empty() && m_parts.empty(); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   01/17
//=======================================================================================
struct GeometryListBuilder
{
private:
    GeometryList                m_geometries;
    Transform                   m_transform;
    DgnDbR                      m_dgndb;
    DgnElementId                m_elementId;
    mutable DisplayParamsCache  m_displayParamsCache;
    bool                        m_surfacesOnly;
    bool                        m_haveTransform;

    bool AddGeometry(IGeometryR geom, bool isCurved, DisplayParamsCR displayParams, TransformCR transform);
    bool AddGeometry(IGeometryR geom, bool isCurved, DisplayParamsCR displayParams, TransformCR transform, DRange3dCR range);
public:
    GeometryListBuilder(DgnDbR db, TransformCR transform, bool surfacesOnly) : m_transform(transform), m_dgndb(db), m_surfacesOnly(surfacesOnly), m_haveTransform(!transform.IsIdentity()) { }
    explicit GeometryListBuilder(DgnDbR db, bool surfacesOnly=false) : m_transform(Transform::FromIdentity()), m_dgndb(db), m_surfacesOnly(surfacesOnly), m_haveTransform(false) { }

    void AddGeometry(GeometryR geom) { m_geometries.push_back(&geom); }
    void SetGeometryList(GeometryList const& geometries) { m_geometries = geometries; }

    DGNPLATFORM_EXPORT bool Add(CurveVectorR curves, bool filled, DisplayParamsCR displayParams, TransformCR transform);
    DGNPLATFORM_EXPORT bool Add(ISolidPrimitiveR primitive, DisplayParamsCR displayParams, TransformCR transform);
    DGNPLATFORM_EXPORT bool Add(RefCountedMSBsplineSurface& surface, DisplayParamsCR displayParams, TransformCR transform);
    DGNPLATFORM_EXPORT bool Add(PolyfaceHeaderR polyface, bool filled, DisplayParamsCR displayParams, TransformCR transform);
    DGNPLATFORM_EXPORT bool Add(IBRepEntityR body, DisplayParamsCR displayParams, TransformCR transform);
    DGNPLATFORM_EXPORT bool Add(TextStringR textString, DisplayParamsCR displayParams, TransformCR transform);

    bool IsEmpty() const { return m_geometries.empty(); }
    void Clear() { m_geometries.clear(); }
    GeometryList const& GetGeometries() const { return m_geometries; }
    GeometryList& GetGeometries() { return m_geometries; }

    DgnElementId GetElementId() const { return m_elementId; }
    void SetElementId(DgnElementId id) { m_elementId = id; }

    TransformCR GetTransform() const { return m_transform; }
    void SetTransform(TransformCR tf) { m_transform = tf; m_haveTransform = !m_transform.IsIdentity(); }

    DgnDbR GetDgnDb() const { return m_dgndb; }
    DisplayParamsCacheR GetDisplayParamsCache() const { return m_displayParamsCache; }
    bool WantSurfacesOnly() const { return m_surfacesOnly; }

    //! Convert the geometry accumulated by this builder into a set of meshes.
    DGNPLATFORM_EXPORT MeshList ToMeshes(GeometryOptionsCR options, double tolerance=0.001) const;

    DGNPLATFORM_EXPORT void SaveToGraphicList(bvector<Render::GraphicPtr>& graphics, Render::System const& system, GeometryOptionsCR options, double tolerance=0.001) const;
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
// @bsistruct                                                   Paul.Connelly   03/17
//=======================================================================================
struct MeshArgs : TriMeshArgs
{
    bvector<int32_t>                m_indices;
    bvector<uint32_t>               m_colorTable;

    template<typename T, typename U> void Set(T& ptr, U const& src) { ptr = 0 != src.size() ? src.data() : nullptr; }

    template<typename T, typename U> void Set(int32_t& count, T& ptr, U const& src)
        {
        count = static_cast<int32_t>(src.size());
        Set(ptr, src);
        }

    void Clear();
    void Transform(TransformCR);
    bool Init(MeshCR mesh, Render::System const& system, DgnDbR db);
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct IndexedPolyline : IndexedPolylineArgs::Polyline
{
    bool IsValid() const { return 0 < m_numIndices; }

    void Reset()
        {
        m_numIndices = 0;
        m_vertIndex = nullptr;
        }

    bool Init(PolylineCR line)
        {
        Reset();

        m_numIndices = static_cast<uint32_t>(line.GetIndices().size());
        m_vertIndex = &line.GetIndices()[0];

        return IsValid();
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct PolylineArgs : IndexedPolylineArgs
{
    bvector<IndexedPolyline>    m_polylines;
    bvector<uint32_t>           m_colorTable;

    bool IsValid() const { return !m_polylines.empty(); }

    void Reset();
    bool Init(MeshCR mesh);
    void Transform(TransformCR tf);
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   03/17
//=======================================================================================
struct PrimitiveBuilder : GraphicBuilder
{
protected:
    System&             m_system;
    GeometryListBuilder m_geomList;
    bvector<GraphicPtr> m_primitives;
    GraphicParams       m_graphicParams;
    GeometryParams      m_geometryParams;
    bool                m_geometryParamsValid = false;
    bool                m_isOpen = true;

    DGNPLATFORM_EXPORT void _ActivateGraphicParams(GraphicParamsCR, Render::GeometryParamsCP) override;
    DGNPLATFORM_EXPORT void _AddTile(Render::TextureCR tile, TileCorners const& corners) override;
    DGNPLATFORM_EXPORT void _AddSubGraphic(Render::Graphic&, TransformCR, Render::GraphicParamsCR, ClipVectorCP clip) override;
    DGNPLATFORM_EXPORT Render::GraphicBuilderPtr _CreateSubGraphic(TransformCR, ClipVectorCP clip) const override;

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
    DGNPLATFORM_EXPORT bool _IsOpen() const override { return m_isOpen; }
    DGNPLATFORM_EXPORT Render::GraphicPtr _Finish() override;

    void AddTriMesh(TriMeshArgsCR args);

    GraphicParamsCR GetGraphicParams() const { return m_graphicParams; }
    GeometryParamsCP GetGeometryParams() const { return m_geometryParamsValid ? &m_geometryParams : nullptr; }
    DisplayParamsCR GetDisplayParams(bool ignoreLighting=false) const;
public:
    PrimitiveBuilder(System& system, Render::GraphicBuilder::CreateParams const& params)
        : GraphicBuilder(params), m_system(system), m_geomList(params.m_dgndb) { }
};

END_BENTLEY_RENDER_PRIMITIVES_NAMESPACE


/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/RenderPrimitives.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

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
DEFINE_POINTER_SUFFIX_TYPEDEFS(GeometryList);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Polyface);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Strokes);
DEFINE_POINTER_SUFFIX_TYPEDEFS(GeometryCollection);
DEFINE_POINTER_SUFFIX_TYPEDEFS(VertexKey);
DEFINE_POINTER_SUFFIX_TYPEDEFS(TriangleKey);
DEFINE_POINTER_SUFFIX_TYPEDEFS(GeomPart);
DEFINE_POINTER_SUFFIX_TYPEDEFS(GeometryOptions);
DEFINE_POINTER_SUFFIX_TYPEDEFS(GeometryAccumulator);
DEFINE_POINTER_SUFFIX_TYPEDEFS(ColorTable);
DEFINE_POINTER_SUFFIX_TYPEDEFS(PrimitiveBuilder);
DEFINE_POINTER_SUFFIX_TYPEDEFS(QVertex3d);

DEFINE_REF_COUNTED_PTR(DisplayParams);
DEFINE_REF_COUNTED_PTR(MeshPart);
DEFINE_REF_COUNTED_PTR(Mesh);
DEFINE_REF_COUNTED_PTR(MeshBuilder);
DEFINE_REF_COUNTED_PTR(Geometry);
DEFINE_REF_COUNTED_PTR(GeomPart);

typedef bvector<MeshInstance>       MeshInstanceList;
typedef bvector<MeshPartPtr>        MeshPartList;
typedef bvector<Triangle>           TriangleList;
typedef bvector<Polyline>           PolylineList;
typedef bvector<Polyface>           PolyfaceList;
typedef bvector<Strokes>            StrokesList;
typedef bmap<double, PolyfaceList>  PolyfaceMap;

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
    enum class SurfacesOnly { Yes, No };

    NormalMode          m_normalMode;
    SurfacesOnly        m_surfaces;

    explicit GeometryOptions(NormalMode normals=NormalMode::Always, SurfacesOnly surfaces=SurfacesOnly::No)
        : m_normalMode(normals), m_surfaces(surfaces) { }

    bool WantSurfacesOnly() const { return SurfacesOnly::Yes == m_surfaces; }
};

//=======================================================================================
//! Describes the appearance of a Geometry.
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct DisplayParams : RefCountedBase
{
    friend struct DisplayParamsCache;

    enum class Type { Mesh, Linear, Text };
private:
    DgnCategoryId       m_categoryId;
    DgnSubCategoryId    m_subCategoryId;
    TexturePtr          m_texture; // meshes only
    MaterialPtr         m_material; // meshes only
    DgnMaterialCPtr     m_dgnMaterial;
    RenderingAssetCP    m_renderingAsset = nullptr;
    GradientSymbCPtr    m_gradient;
    DgnMaterialId       m_materialId;
    ColorDef            m_lineColor = ColorDef::White(); // all types of geometry (edge color for meshes)
    ColorDef            m_fillColor = ColorDef::White(); // meshes only
    uint32_t            m_width = 0; // linear and mesh (edges)
    LinePixels          m_linePixels = LinePixels::Solid; // linear and mesh (edges)
    FillFlags           m_fillFlags = FillFlags::None; // meshes only
    DgnGeometryClass    m_class = DgnGeometryClass::Primary;
    bool                m_ignoreLighting = false; // always true for text and linear geometry; true for meshes only if normals not desired
    bool                m_resolved = true;

    virtual uint32_t _GetExcessiveRefCountThreshold() const override { return 0x7fffffff; }

    DisplayParamsCPtr Clone() const;
    
    DGNPLATFORM_EXPORT DisplayParams(Type, GraphicParamsCR, GeometryParamsCP, bool filled);
    DisplayParams(DisplayParamsCR rhs) = default;
    void Resolve(DgnDbR, System&);
public:
    ColorDef GetFillColorDef() const { return m_fillColor; }
    uint32_t GetFillColor() const { return GetFillColorDef().GetValue(); }
    ColorDef GetLineColorDef() const { return m_lineColor; }
    uint32_t GetLineColor() const { return GetLineColorDef().GetValue(); }

    TextureP GetTexture() const { return m_texture.get(); }
    MaterialP GetMaterial() const { return m_material.get(); }
    uint32_t GetLineWidth() const { return m_width; }
    LinePixels GetLinePixels() const { return m_linePixels; }
    FillFlags GetFillFlags() const { return m_fillFlags; }

    DgnGeometryClass GetClass() const { return m_class; }
    DgnCategoryId GetCategoryId() const { return m_categoryId; }
    DgnSubCategoryId GetSubCategoryId() const { return m_subCategoryId; }
    DgnMaterialId GetMaterialId() const { return m_materialId; }
    RenderingAssetCP GetRenderingAsset() const { return m_renderingAsset; }

    bool IgnoresLighting() const { return m_ignoreLighting; }
    bool HasFillTransparency() const { return 0 != GetFillColorDef().GetAlpha(); }
    bool HasLineTransparency() const { return 0 != GetLineColorDef().GetAlpha(); }
    bool IsTextured() const { BeAssert(m_resolved); return nullptr != GetTexture(); }
    bool NeverRegionOutline() const { return 0 != ((int) FillFlags::Blanking & (int) GetFillFlags()) || (m_gradient.IsValid() && !m_gradient->GetIsOutlined()); }
    bool HasRegionOutline() const;

    enum class ComparePurpose
    {
        Merge,  //!< ignores category, subcategory, class, and considers colors equivalent if both have or both lack transparency
        Strict  //!< compares all members
    };

    DGNPLATFORM_EXPORT bool IsLessThan(DisplayParamsCR rhs, ComparePurpose purpose=ComparePurpose::Strict) const;
    DGNPLATFORM_EXPORT bool IsEqualTo(DisplayParamsCR rhs, ComparePurpose purpose=ComparePurpose::Strict) const;

    static DisplayParamsCPtr CreateForMesh(GraphicParamsCR gf, GeometryParamsCP geom, bool filled, DgnDbR db, System& sys)
        {
        DisplayParamsPtr dp = new DisplayParams(Type::Mesh, gf, geom, filled);
        dp->Resolve(db, sys);
        return dp;
        }
    static DisplayParamsCPtr CreateForLinear(GraphicParamsCR gf, GeometryParamsCP geom)
        {
        return new DisplayParams(Type::Linear, gf, geom, false);
        }
    static DisplayParamsCPtr CreateForText(GraphicParamsCR gf, GeometryParamsCP geom)
        {
        return new DisplayParams(Type::Text, gf, geom, false);
        }
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
        DisplayParams ndp(type, gf, geom, filled);
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

    FeatureTableCR  FeatureTable() const { return m_features; }
    FeatureTableR  FeatureTable()  { return m_features; }
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
    uint8_t     m_edgeFlags[3];

    uint32_t operator[](int index) const { BeAssert(index < 3); return m_indices[index]; }
    uint32_t& operator[](int index) { BeAssert(index < 3); return m_indices[index]; }

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
    bool GetEdgeVisible(size_t index) const { BeAssert(index < 3); return m_edgeFlags[index] == MeshEdge::Flags::Visible; }

    bool IsDegenerate() const  { return m_indices[0] == m_indices[1] || m_indices[0] == m_indices[2] || m_indices[1] == m_indices[2]; }                                   
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Polyline
{
private:
    bvector<uint32_t>   m_indices;
    float               m_startDistance;
    FPoint3d            m_rangeCenter;

public:
    Polyline () : m_startDistance(0.0) { }
    Polyline (float startDistance, FPoint3dCR rangeCenter) : m_startDistance(startDistance), m_rangeCenter(rangeCenter) { }
    bvector<uint32_t> const& GetIndices() const { return m_indices; }
    bvector<uint32_t>& GetIndices() { return m_indices; }
    float GetStartDistance() const { return m_startDistance; }
    FPoint3dCR GetRangeCenter() const { return m_rangeCenter; }

    void AddIndex(uint32_t index)  { if (m_indices.empty() || m_indices.back() != index) m_indices.push_back(index); }
    void Clear() { m_indices.clear(); }
};

//=======================================================================================
//! Represents a possibly-quantized position. Used during mesh generation.
//! See QVertex3dList.
// @bsistruct                                                   Paul.Connelly   05/17
//=======================================================================================
struct QVertex3d
{
    using Params = QPoint3d::Params;
    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(Params);
private:
    struct Quantized { uint16_t x, y, z; };
    struct Unquantized { float x, y, z; };

    union
        {
        Quantized   m_q;
        Unquantized m_u;
        };
    bool    m_quantized;
public:
    QVertex3d() { }
    QVertex3d(DPoint3dCR dpt, ParamsCR params) : QVertex3d(FPoint3d::From(dpt), params) { }
    QVertex3d(FPoint3dCR fpt, ParamsCR params)
        {
        double x, y=0, z=0; // Compiler doesn't realize y and z are never used unless they are initialized, therefore must initialize here...
        m_quantized = (Quantization::IsInRange(x = Quantization::QuantizeDouble(fpt.x, params.origin.x, params.scale.x))
                    && Quantization::IsInRange(y = Quantization::QuantizeDouble(fpt.y, params.origin.y, params.scale.y))
                    && Quantization::IsInRange(z = Quantization::QuantizeDouble(fpt.z, params.origin.z, params.scale.z)));
        if (m_quantized)
            {
            m_q.x = static_cast<uint16_t>(x);
            m_q.y = static_cast<uint16_t>(y);
            m_q.z = static_cast<uint16_t>(z);
            }
        else
            {
            *reinterpret_cast<FPoint3dP>(&m_u) = fpt;
            }
        }

    bool IsQuantized() const { return m_quantized; }
    QPoint3dCR GetQPoint3d() const { BeAssert(IsQuantized()); return *reinterpret_cast<QPoint3dCP>(&m_q); }
    FPoint3dCR GetFPoint3d() const { BeAssert(!IsQuantized()); return *reinterpret_cast<FPoint3dCP>(&m_u); }

    FPoint3d Unquantize(ParamsCR params) const
        {
        return IsQuantized() ? GetQPoint3d().Unquantize32(params) : GetFPoint3d();
        }
};

//=======================================================================================
//! A list of possibly-quantized positions. When mesh generation begins, we typically
//! know a range that will contain most of the mesh's vertices, but vertices of triangles
//! which only partially intersect that range must also be included. Vertices within the
//! initial range are quantized to that range; vertices outside of it are stored directly,
//! and used to extend the initial range.
//! After mesh generation completes, the entire list is requantized to the actual range
//! if necessary.         
// @bsistruct                                                   Paul.Connelly   05/17
//=======================================================================================
struct QVertex3dList
{
private:
    bvector<FPoint3d>   m_fpoints;
    QPoint3dList        m_qpoints;
    DRange3d            m_range;
public:
    explicit QVertex3dList(DRange3dCR range) : m_qpoints(range), m_range(range) { }

    void Add(QVertex3dCR vertex)
        {
        if (!vertex.IsQuantized())
            {
            FPoint3dCR fpt = vertex.GetFPoint3d();
            m_fpoints.push_back(fpt);
            m_range.Extend(fpt);
            }
        else if (m_fpoints.empty())
            {
            m_qpoints.push_back(vertex.GetQPoint3d());
            }
        else
            {
            m_fpoints.push_back(vertex.Unquantize(m_qpoints.GetParams()));
            }
        }

    //! If any unquantized vertices exist, requantize. IsFullyQuantized() returns true after this operation.
    DGNPLATFORM_EXPORT void Requantize();
    bool IsFullyQuantized() const { return m_fpoints.empty(); }
    QPoint3dListCR GetQuantizedPoints() const { BeAssert(IsFullyQuantized()); return m_qpoints; }
    QPoint3d::ParamsCR GetParams() const { return m_qpoints.GetParams(); }
    bool empty() const { return m_fpoints.empty() && m_qpoints.empty(); }
    size_t size() const { return m_fpoints.size() + m_qpoints.size(); }

    //! Returns the accumulated range, which may be larger than the initial range passed to the constructor.
    DRange3dCR GetRange() const { return m_range; }
    void Init(DRange3dCR range, QPoint3dCP points, size_t nPoints); 
};

DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(QVertex3dList);

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

    DisplayParamsCPtr               m_displayParams;
    TriangleList                    m_triangles;
    PolylineList                    m_polylines;
    QVertex3dList                   m_verts;
    QPoint3dList                    m_normals;                          
    bvector<FPoint2d>               m_uvParams;
    ColorTable                      m_colorTable;
    bvector<uint16_t>               m_colors;
    Features                        m_features;
    PrimitiveType                   m_type;
    mutable MeshEdgesPtr            m_edges;

    Mesh(DisplayParamsCR params, FeatureTableP featureTable, PrimitiveType type, DRange3dCR range)
        : m_displayParams(&params), m_features(featureTable), m_type(type), m_verts(range), m_normals(QPoint3d::Params::FromNormalizedRange()) { }

    friend struct MeshBuilder;
    void SetDisplayParams(DisplayParamsCR params) { m_displayParams = &params; }
public:
    static MeshPtr Create(DisplayParamsCR params, FeatureTableP featureTable, PrimitiveType type, DRange3dCR range) { return new Mesh(params, featureTable, type, range); }

    DPoint3d                        GetPoint(uint32_t index) const;
    DGNPLATFORM_EXPORT DRange3d     GetTriangleRange(TriangleCR triangle) const;
    DGNPLATFORM_EXPORT DVec3d       GetTriangleNormal(TriangleCR triangle) const;
    DGNPLATFORM_EXPORT bool         HasNonPlanarNormals() const;

    DisplayParamsCR                 GetDisplayParams() const { return *m_displayParams; } //!< The mesh symbology
    TriangleList const&             Triangles() const { return m_triangles; } //!< Triangles defined as a set of 3 indices into the vertex attribute arrays.
    PolylineList const&             Polylines() const { return m_polylines; } //!< Polylines defined as a set of indices into the vertex attribute arrays.
    QPoint3dListCR                  Points() const { return m_verts.GetQuantizedPoints(); } //!< Position vertex attribute array
    QVertex3dListCR                 Verts() const { return m_verts; }
    QVertex3dListR                  VertsR() { return m_verts; }
    QPoint3dListCR                  Normals() const { return m_normals; }   //!< Normal vertex attribute array
    QPoint3dListR                   NormalsR()  { return m_normals; }       //!< Normal vertex attribute array
    bvector<FPoint2d> const&        Params() const { return m_uvParams; }   //!< UV params vertex attribute array
    bvector<FPoint2d>&              ParamsR() { return m_uvParams; }        //!< UV params vertex attribute array
    bvector<uint16_t> const&        Colors() const { return m_colors; }     //!< Vertex attribute array specifying an index into the color table
    bvector<uint16_t>&              ColorsR() { return m_colors; }          //!< Vertex attribute array specifying an index into the color table
    ColorTableCR                    GetColorTable() const { return m_colorTable; }
    ColorTableR                     GetColorTableR() { return m_colorTable; }
    void                            ToFeatureIndex(FeatureIndex& index) const { m_features.ToFeatureIndex(index); }
    MeshEdgesPtr                    GetEdges() const { return m_edges; }

    bool IsEmpty() const { return m_triangles.empty() && m_polylines.empty(); }
    PrimitiveType GetType() const { return m_type; }

    DGNPLATFORM_EXPORT DRange3d GetRange() const;
    DGNPLATFORM_EXPORT DRange3d GetUVRange() const;

    void Close() { m_verts.Requantize(); }

    void AddTriangle(TriangleCR triangle) { BeAssert(PrimitiveType::Mesh == GetType()); m_triangles.push_back(triangle); }
    void AddPolyline(PolylineCR polyline) { BeAssert(PrimitiveType::Polyline == GetType() || PrimitiveType::Point == GetType()); m_polylines.push_back(polyline); }
    uint32_t AddVertex(QVertex3dCR vertex, QPoint3dCP normal, DPoint2dCP param, uint32_t fillColor, FeatureCR feature);
    void GetGraphics (bvector<Render::GraphicPtr>& graphics, Dgn::Render::SystemCR system, struct GetMeshGraphicsArgs& args, DgnDbR db) const;
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
    QPoint3d        m_normal;
    DPoint2d        m_param;
    uint32_t        m_fillColor = 0;
    Feature         m_feature;
    QVertex3d       m_position;
    bool            m_normalValid;
    bool            m_paramValid;

    VertexKey() { }
    VertexKey(DPoint3dCR point, FeatureCR feature, uint32_t fillColor, QPoint3d::ParamsCR qParams, DVec3dCP normal=nullptr, DPoint2dCP param=nullptr);

    QPoint3dCP GetNormal() const { return m_normalValid ? &m_normal : nullptr; }
    DPoint2dCP GetParam() const { return m_paramValid ? &m_param : nullptr; }

    //=======================================================================================
    // @bsistruct                                                   Paul.Connelly   12/16
    //=======================================================================================
    struct Comparator
    {
        DGNPLATFORM_EXPORT bool operator()(VertexKey const& lhs, VertexKey const& rhs) const;
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
    struct Polyface : RefCountedBase
        {
        PolyfaceQueryCR             m_polyface;
        MeshEdgeCreationOptions     m_edgeOptions;
        size_t                      m_baseTriangleIndex;
        bmap<uint32_t, uint32_t>    m_vertexIndexMap;           // Map from the Mesh vertex index to the polyface vertex index.

        Polyface(PolyfaceQueryCR polyface, MeshEdgeCreationOptionsCR edgeOptions, size_t baseTriangleIndex) : m_polyface(polyface), m_edgeOptions(edgeOptions), m_baseTriangleIndex(baseTriangleIndex) { }
        };

private:
    MeshPtr                         m_mesh;
    VertexMap                       m_clusteredVertexMap;
    VertexMap                       m_unclusteredVertexMap;
    TriangleSet                     m_triangleSet;
    double                          m_tolerance;
    double                          m_areaTolerance;
    RefCountedPtr<Polyface>         m_currentPolyface;
    DRange3d                        m_tileRange;


    MeshBuilder(DisplayParamsCR params, double tolerance, double areaTolerance, FeatureTableP featureTable, Mesh::PrimitiveType type, DRange3dCR range)
        : m_mesh(Mesh::Create(params, featureTable, type, range)), m_tolerance(tolerance), m_areaTolerance(areaTolerance), m_tileRange(range) { }

    uint32_t AddVertex(VertexMap& vertices, VertexKeyCR vertex);
public:
    static MeshBuilderPtr Create(DisplayParamsCR params, double tolerance, double areaTolerance, FeatureTableP featureTable, Mesh::PrimitiveType type, DRange3dCR range)
        { return new MeshBuilder(params, tolerance, areaTolerance, featureTable, type, range); }

    DGNPLATFORM_EXPORT void AddTriangle(PolyfaceVisitorR visitor, RenderingAssetCP, DgnDbR dgnDb, FeatureCR feature, bool doVertexClustering, bool includeParams, uint32_t fillColor);
    DGNPLATFORM_EXPORT void AddPolyline(bvector<DPoint3d>const& polyline, FeatureCR feature, bool doVertexClustering, uint32_t fillColor, double startDistance, DPoint3dCR rangeCenter);
    DGNPLATFORM_EXPORT void BeginPolyface(PolyfaceQueryCR polyface, MeshEdgeCreationOptionsCR options);
    DGNPLATFORM_EXPORT void EndPolyface();

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
    bool                m_displayEdges = true;

    Polyface(DisplayParamsCR displayParams, PolyfaceHeaderR polyface, bool displayEdges = true) : m_displayParams(&displayParams), m_polyface(&polyface), m_displayEdges(displayEdges) { }

    void Transform(TransformCR transform) { if (m_polyface.IsValid()) m_polyface->Transform(transform); }
    Polyface Clone() const { return Polyface(*m_displayParams, *m_polyface->Clone(), m_displayEdges); }
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
        DPoint3d            m_rangeCenter;

        PointList(double startDistance, DPoint3dCR rangeCenter) : m_startDistance(startDistance), m_rangeCenter(rangeCenter) { }
        PointList() : m_startDistance(0.0), m_rangeCenter(DPoint3d::FromZero()) { }
        PointList(bvector<DPoint3d>&& points, DPoint3dCR rangeCenter) : m_startDistance(0.0), m_points(std::move(points)), m_rangeCenter(rangeCenter) { }
        };


    typedef bvector<PointList> PointLists;

    DisplayParamsCPtr   m_displayParams;
    PointLists          m_strokes;
    bool                m_disjoint;

    Strokes(DisplayParamsCR displayParams, PointLists&& strokes, bool disjoint) : m_displayParams(&displayParams), m_strokes(std::move(strokes)), m_disjoint(disjoint) { }
    Strokes(DisplayParamsCR displayParams, bool disjoint) : m_displayParams(&displayParams), m_disjoint(disjoint) { }

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
    static GeometryPtr Create(TextStringR textString, TransformCR transform, DRange3dCR range, DgnElementId entityId, DisplayParamsCR params, DgnDbR db, bool checkGlyphBoxes);
    //! Create a Geometry for a part instance.
    static GeometryPtr Create(GeomPartR part, TransformCR transform, DRange3dCR range, DgnElementId entityId, DisplayParamsCR params, DgnDbR db);
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
    void SetInCache(bool inCache);
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct GeometryCollection
{
private:
    MeshList            m_meshes;
    MeshPartList        m_parts;
    bool                m_isComplete = true;
    bool                m_curved = false;
public:
    MeshList& Meshes()              { return m_meshes; }
    MeshList const& Meshes() const  { return m_meshes; }
    MeshPartList& Parts()           { return m_parts; }
    bool IsEmpty() const            { return m_meshes.empty() && m_parts.empty(); }
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
    DRange3d                    m_tileRange;

    bool AddGeometry(IGeometryR geom, bool isCurved, DisplayParamsCR displayParams, TransformCR transform);
    bool AddGeometry(IGeometryR geom, bool isCurved, DisplayParamsCR displayParams, TransformCR transform, DRange3dCR range);
public:
    GeometryAccumulator(DgnDbR db, System& system, TransformCR transform, DRange3dCR tileRange, bool surfacesOnly) : m_transform(transform), m_tileRange(tileRange), m_displayParamsCache(db, system), m_surfacesOnly(surfacesOnly), m_haveTransform(!transform.IsIdentity()) { }
    GeometryAccumulator(DgnDbR db, System& system, bool surfacesOnly=false) : m_transform(Transform::FromIdentity()), m_displayParamsCache(db, system), m_surfacesOnly(surfacesOnly), m_haveTransform(false), m_tileRange(DRange3d::NullRange()) { }

    void AddGeometry(GeometryR geom) { m_geometries.push_back(geom); }
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

    DgnDbR GetDgnDb() const { return m_displayParamsCache.GetDgnDb(); }
    System& GetSystem() const { return m_displayParamsCache.GetSystem(); }
    DisplayParamsCacheR GetDisplayParamsCache() const { return m_displayParamsCache; }
    bool WantSurfacesOnly() const { return m_surfacesOnly; }

    //! Convert the geometry accumulated by this builder into a set of meshes.
    DGNPLATFORM_EXPORT MeshList ToMeshes(GeometryOptionsCR options, double tolerance=0.001) const;

    //! Populate a list of Graphic objects from the accumulated Geometry objects.
    DGNPLATFORM_EXPORT void SaveToGraphicList(bvector<Render::GraphicPtr>& graphics, GeometryOptionsCR options,double tolerance=0.001) const;

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

    template<typename T, typename U> void Set(T& ptr, U const& src) { ptr = (0 != src.size() ? src.data() : nullptr); }

    template<typename T, typename U> void Set(int32_t& count, T& ptr, U const& src)
        {
        count = static_cast<int32_t>(src.size());
        Set(ptr, src);
        }

    void Clear();
    bool Init(MeshCR mesh);
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
        m_startDistance = 0.0;
        }

    bool Init(PolylineCR line) { return Init(line.GetIndices(), line.GetStartDistance(), line.GetRangeCenter()); }
        
    bool Init (bvector<uint32_t> const& indices, double startDistance, FPoint3dCR rangeCenter)
        {
        Reset();

        m_numIndices = static_cast<uint32_t>(indices.size());
        m_vertIndex = &indices[0];
        m_startDistance = startDistance;
        m_rangeCenter = rangeCenter;

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
    void FinishInit(MeshCR mesh);
};


//=======================================================================================
// @bsistruct                                                   Ray.Bentley     05/2017
//=======================================================================================
struct ElementMeshEdgeArgs : MeshEdgeArgs
{
    bvector<uint32_t>               m_colorTable;

    bool Init(MeshCR mesh);
};


//=======================================================================================
// @bsistruct                                                   Ray.Bentley     05/2017
//=======================================================================================
struct ElementSilhouetteEdgeArgs : SilhouetteEdgeArgs
{
    bvector<uint32_t>               m_colorTable;

    bool Init(MeshCR mesh);
};


//=======================================================================================
// @bsistruct                                                   Ray.Bentley     05/2017
//=======================================================================================
struct ElementPolylineEdgeArgs : PolylineArgs
{
    bool Init(MeshCR mesh);
};


//=======================================================================================
// @bsistruct                                                   Ray.Bentley     05/2017
//=======================================================================================
struct GetMeshGraphicsArgs
{
    PolylineArgs                     m_polylineArgs;
    MeshArgs                         m_meshArgs;
    ElementMeshEdgeArgs              m_visibleEdgesArgs;
    ElementSilhouetteEdgeArgs        m_invisibleEdgesArgs;
    ElementPolylineEdgeArgs          m_polylineEdgesArgs;
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
        : GraphicBuilder(params), m_accum(params.m_dgndb, system) { m_accum.SetElementId(elemId); m_accum.SetTransform(accumulatorTf); }

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
    DGNPLATFORM_EXPORT void _AddTriMesh(TriMeshArgs const& args) override { } // WIP.
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
    void SetCheckGlyphBoxes(bool check) { m_accum.SetCheckGlyphBoxes(check); }
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

    void Add(GeometryR geom) { m_accum.AddGeometry(geom); }

    //! Reset this builder for reuse.
    DGNPLATFORM_EXPORT void ReInitialize(TransformCR localToWorld, TransformCR accumulatorTransform=Transform::FromIdentity(), DgnElementId elemId=DgnElementId());
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

    DGNPLATFORM_EXPORT void _AddTile(Render::TextureCR tile, TileCorners const& corners) override;
    DGNPLATFORM_EXPORT void _AddSubGraphic(Render::Graphic&, TransformCR, Render::GraphicParamsCR, ClipVectorCP clip) override;
    DGNPLATFORM_EXPORT Render::GraphicBuilderPtr _CreateSubGraphic(TransformCR, ClipVectorCP clip) const override;
    DGNPLATFORM_EXPORT Render::GraphicPtr _FinishGraphic(GeometryAccumulatorR) override;
    void _Reset() override { m_primitives.clear(); }

    void AddTriMesh(TriMeshArgsCR args);
public:
    PrimitiveBuilder(System& system, Render::GraphicBuilder::CreateParams const& params) : GeometryListBuilder(system, params) { }
};

END_BENTLEY_RENDER_PRIMITIVES_NAMESPACE


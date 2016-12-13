/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ElementTileTree.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_END__

#include <DgnPlatform/TileTree.h>
#include <DgnPlatform/DgnModel.h>
#include <DgnPlatform/DgnElement.h>

#define BEGIN_ELEMENT_TILETREE_NAMESPACE BEGIN_BENTLEY_DGN_NAMESPACE namespace ElementTileTree {
#define END_ELEMENT_TILETREE_NAMESPACE } END_BENTLEY_DGN_NAMESPACE
#define USING_NAMESPACE_ELEMENT_TILETREE using namespace BentleyApi::Dgn::ElementTileTree;

BEGIN_ELEMENT_TILETREE_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(Tile);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Root);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Loader);
DEFINE_POINTER_SUFFIX_TYPEDEFS(DisplayParams);
DEFINE_POINTER_SUFFIX_TYPEDEFS(TextureImage);
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
DEFINE_POINTER_SUFFIX_TYPEDEFS(Context);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Result);
DEFINE_POINTER_SUFFIX_TYPEDEFS(VertexKey);
DEFINE_POINTER_SUFFIX_TYPEDEFS(TriangleKey);
DEFINE_POINTER_SUFFIX_TYPEDEFS(GeomPart);

DEFINE_REF_COUNTED_PTR(Tile);
DEFINE_REF_COUNTED_PTR(DisplayParams);
DEFINE_REF_COUNTED_PTR(TextureImage);
DEFINE_REF_COUNTED_PTR(MeshPart);
DEFINE_REF_COUNTED_PTR(Mesh);
DEFINE_REF_COUNTED_PTR(MeshBuilder);
DEFINE_REF_COUNTED_PTR(Geometry);
DEFINE_REF_COUNTED_PTR(GeomPart);
DEFINE_REF_COUNTED_PTR(Root);
DEFINE_REF_COUNTED_PTR(Loader);

typedef bvector<MeshPtr>            MeshList;
typedef bvector<MeshInstance>       MeshInstanceList;
typedef bvector<MeshPartPtr>        MeshPartList;
typedef bvector<TilePtr>            TileList;
typedef bvector<TileP>              TilePList;
typedef bvector<GeometryPtr>        GeometryList;
typedef bvector<Triangle>           TriangleList;
typedef bvector<Polyline>           PolylineList;
typedef bvector<Polyface>           PolyfaceList;
typedef bvector<Strokes>            StrokesList;
typedef bmap<double, PolyfaceList> PolyfaceMap;

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
struct TextureImage : RefCountedBase
{
private:
    ImageSource     m_imageSource;

    TextureImage(ImageSource&& imageSource) : m_imageSource(std::move(imageSource)) { BeAssert(m_imageSource.IsValid()); }
public:
    static TextureImagePtr Create(ImageSource&& imageSource) { return new TextureImage(std::move(imageSource)); }
    static TextureImagePtr Create(ImageSourceCR imageSource) { return Create(ImageSource(imageSource)); }
    static ImageSource Load(DisplayParamsCR params, DgnDbR db);

    ImageSourceCR GetImageSource() const { return m_imageSource; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct DisplayParams : RefCountedBase
{
private:
    DgnCategoryId           m_categoryId;
    DgnSubCategoryId        m_subCategoryId;
    uint32_t                m_fillColor;
    uint32_t                m_rasterWidth;
    DgnMaterialId           m_materialId;
    mutable TextureImagePtr m_textureImage;
    bool                    m_ignoreLighting;

    DisplayParams(GraphicParamsCP graphicParams, GeometryParamsCP geometryParams, bool ignoreLighting);
    DisplayParams(uint32_t fillColor, TextureImageP texture, bool ignoreLighting) : m_fillColor(fillColor), m_textureImage(texture), m_ignoreLighting(ignoreLighting), m_rasterWidth(0) { }
    DisplayParams(uint32_t fillColor, GeometryParamsCR geometryParams) : m_fillColor(fillColor), m_ignoreLighting(false), m_materialId(geometryParams.GetMaterialId()), m_rasterWidth(0) {}
    DisplayParams(uint32_t fillColor, DgnMaterialId materialId) : m_fillColor(fillColor), m_materialId(materialId), m_ignoreLighting(false), m_rasterWidth(0) {}
public:
    static DisplayParamsPtr Create() { return Create(nullptr, nullptr); }
    static DisplayParamsPtr Create(GraphicParamsCR graphicParams, GeometryParamsCR geometryParams, bool ignoreLighting = false) { return Create(&graphicParams, &geometryParams, ignoreLighting); }
    static DisplayParamsPtr Create(GraphicParamsCP graphicParams, GeometryParamsCP geometryParams, bool ignoreLighting = false) { return new DisplayParams(graphicParams, geometryParams, ignoreLighting); }
    static DisplayParamsPtr Create(uint32_t fillColor, TextureImageP textureImage, bool ignoreLighting) { return new DisplayParams(fillColor, textureImage, ignoreLighting); }
    static DisplayParamsPtr Create(uint32_t fillColor, GeometryParamsCR geometryParams) { return new DisplayParams(fillColor, geometryParams); }
    static DisplayParamsPtr Create(uint32_t fillColor, DgnMaterialId materialId) { return new DisplayParams(fillColor, materialId); }

    DGNPLATFORM_EXPORT bool operator<(DisplayParamsCR rhs) const;

    DgnCategoryId GetCategoryId() const { return m_categoryId; }
    DgnSubCategoryId GetSubCategoryId() const { return m_subCategoryId; }
    DgnMaterialId GetMaterialId() const { return m_materialId; }
    uint32_t GetFillColor() const { return m_fillColor; }
    uint32_t GetRasterWidth() const { return m_rasterWidth; }
    bool GetIgnoreLighting() const { return m_ignoreLighting; }
    DgnTextureCPtr QueryTexture(DgnDbR db) const;
    TextureImagePtr& TextureImage() { return m_textureImage; }
    TextureImageCP GetTextureImage() const { return m_textureImage.get(); }
    DGNPLATFORM_EXPORT void ResolveTextureImage(DgnDbR db) const;
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct MeshInstance
{
private:
    MeshPartCPtr        m_part;
    Transform           m_transform;
public:
    MeshInstance(MeshPartCP part, TransformCR transform) : m_part(part), m_transform(transform) { }

    MeshPartCP GetPart() const { return m_part.get(); }
    TransformCR GetTransform() const { return m_transform; }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct MeshPart : RefCountedBase
{
private:
    MeshList    m_meshes;

    MeshPart(MeshList&& meshes) : m_meshes(std::move(meshes)) { }

    virtual uint32_t _GetExcessiveRefCountThreshold() const override { return 100000; }
public:
    static MeshPartPtr Create(MeshList&& meshes) { return new MeshPart(std::move(meshes)); }

    MeshList const& Meshes() const { return m_meshes; }
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
private:
    DisplayParamsPtr        m_displayParams;
    TriangleList            m_triangles;
    PolylineList            m_polylines;
    bvector<DPoint3d>       m_points;
    bvector<DVec3d>         m_normals;
    bvector<DPoint2d>       m_uvParams;
    bvector<DgnElementId>   m_entityIds;
    bool                    m_validIdsPresent = false;

    explicit Mesh(DisplayParamsP params) : m_displayParams(params) { }

    template<typename T> T const* GetMember(bvector<T> const& from, uint32_t at) const { return at < from.size() ? &from[at] : nullptr; }
public:
public:
    static MeshPtr Create(DisplayParamsP params) { return new Mesh(params); }

    DGNPLATFORM_EXPORT DRange3d GetTriangleRange(TriangleCR triangle) const;
    DGNPLATFORM_EXPORT DVec3d GetTriangleNormal(TriangleCR triangle) const;
    DGNPLATFORM_EXPORT bool HasNonPlanarNormals() const;
    DGNPLATFORM_EXPORT bool RemoveEntityGeometry(bset<DgnElementId> const& ids);

    DisplayParamsCP GetDisplayParams() const { return m_displayParams.get(); } //!< The mesh symbology
    DisplayParamsPtr GetDisplayParamsPtr() const { return m_displayParams; } //!< The mesh symbology
    TriangleList const& Triangles() const { return m_triangles; } //!< Triangles defined as a set of 3 indices into the vertex attribute arrays.
    PolylineList const& Polylines() const { return m_polylines; } //!< Polylines defined as a set of indices into the vertex attribute arrays.
    bvector<DPoint3d> const& Points() const { return m_points; } //!< Position vertex attribute array
    bvector<DVec3d> const& Normals() const { return m_normals; } //!< Normal vertex attribute array
    bvector<DPoint2d> const& Params() const { return m_uvParams; } //!< UV params vertex attribute array
    bvector<DgnElementId> const& EntityIds() const { return m_entityIds; } //!< Vertex attribute array specifying the ID of the entity(element or model) from which the vertex was produced
    bvector<DPoint3d>& PointsR() { return m_points; } //!< Position vertex attribute array
    bvector<DVec3d>& NormalsR() { return m_normals; } //!< Normal vertex attribute array
    bvector<DPoint2d>& ParamsR() { return m_uvParams; } //!< UV params vertex attribute array
    bvector<DgnElementId>& EntityIdsR() { return m_entityIds; }

    TriangleCP GetTriangle(uint32_t index) const { return GetMember(m_triangles, index); }
    DPoint3dCP GetPoint(uint32_t index) const { return GetMember(m_points, index); }
    DVec3dCP GetNormal(uint32_t index) const { return GetMember(m_normals, index); }
    DPoint2dCP GetParam(uint32_t index) const { return GetMember(m_uvParams, index); }
    DgnElementId GetEntityId(uint32_t index) const { auto pId = GetMember(m_entityIds, index); return nullptr != pId ? *pId : DgnElementId(); }
    DgnElementId GetTriangleEntityId(TriangleCR triangle) const { return GetEntityId(triangle.m_indices[0]); }
    DgnElementId GetPolylineEntityId(PolylineCR polyline) const { return polyline.GetIndices().empty() ? DgnElementId() : GetEntityId(polyline.GetIndices().front()); }
    bool IsEmpty() const { return m_triangles.empty() && m_polylines.empty(); }
    DRange3d GetRange() const { return DRange3d::From(m_points); }

    DRange3d GetUVRange() const { return DRange3d::From(m_uvParams, 0.0); }

    bool ValidIdsPresent() const { return m_validIdsPresent; }

    void AddTriangle(TriangleCR triangle) { m_triangles.push_back(triangle); }
    void AddPolyline(PolylineCR polyline) { m_polylines.push_back(polyline); }
    DGNPLATFORM_EXPORT void AddMesh(MeshCR mesh);
    uint32_t AddVertex(DPoint3dCR point, DVec3dCP normal, DPoint2dCP param, DgnElementId entityId);
    DGNPLATFORM_EXPORT void SetValidIdsPresent(bool validIdsPresent) { m_validIdsPresent = validIdsPresent; }
};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     06/2016
+===============+===============+===============+===============+===============+======*/
struct MeshMergeKey
{
    DisplayParamsCP m_params;                                                                                                                                                     
    bool            m_hasNormals;
    bool            m_hasFacets;

    MeshMergeKey() : m_params(nullptr), m_hasNormals(false), m_hasFacets(false) { }
    MeshMergeKey(DisplayParamsCR params, bool hasNormals, bool hasFacets) : m_params(&params), m_hasNormals(hasNormals), m_hasFacets(hasFacets) { }
    MeshMergeKey(MeshCR mesh) : MeshMergeKey(*mesh.GetDisplayParams(),  !mesh.Normals().empty(), !mesh.Triangles().empty()) { }

    bool operator<(MeshMergeKey const& rhs) const
        {
        BeAssert(nullptr != m_params && nullptr != rhs.m_params);
        if(m_hasNormals != rhs.m_hasNormals)
            return !m_hasNormals;

        if(m_hasFacets != rhs.m_hasFacets)
            return !m_hasFacets;

        return *m_params < *rhs.m_params;
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct VertexKey
{
    DPoint3d        m_point;
    DVec3d          m_normal;
    DPoint2d        m_param;
    DgnElementId    m_entityId;
    bool            m_normalValid = false;
    bool            m_paramValid = false;

    VertexKey() { }
    VertexKey(DPoint3dCR point, DVec3dCP normal, DPoint2dCP param, DgnElementId entityId) : m_point(point), m_normalValid(nullptr != normal), m_paramValid(nullptr != param), m_entityId(entityId)
        {
        if(m_normalValid) m_normal = *normal;
        if(m_paramValid) m_param = *param;
        }

    DVec3dCP GetNormal() const { return m_normalValid ? &m_normal : nullptr; }
    DPoint2dCP GetParam() const { return m_paramValid ? &m_param : nullptr; }

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
    size_t              m_triangleIndex;
    JsonRenderMaterial  m_material;

    MeshBuilder(DisplayParamsP params, double tolerance, double areaTolerance) : m_mesh(Mesh::Create(params)), m_unclusteredVertexMap(VertexKey::Comparator(1.0E-4)), m_clusteredVertexMap(VertexKey::Comparator(tolerance)), 
            m_tolerance(tolerance), m_areaTolerance(areaTolerance), m_triangleIndex(0) {  }
public:
    static MeshBuilderPtr Create(DisplayParamsP params, double tolerance, double areaTolerance) { return new MeshBuilder(params, tolerance, areaTolerance); }

    DGNPLATFORM_EXPORT void AddTriangle(PolyfaceVisitorR visitor, DgnMaterialId materialId, DgnDbR dgnDb, DgnElementId entityId, bool doVertexClustering, bool duplicateTwoSidedTriangles, bool includeParams);
    DGNPLATFORM_EXPORT void AddPolyline(bvector<DPoint3d>const& polyline, DgnElementId entityId, bool doVertexClustering);
    DGNPLATFORM_EXPORT void AddPolyface(PolyfaceQueryCR polyface, DgnMaterialId materialId, DgnDbR dgnDb, DgnElementId entityId, bool duplicateTwoSidedTriangles, bool includeParams);

    void AddMesh(TriangleCR triangle);
    void AddTriangle(TriangleCR triangle);
    uint32_t AddClusteredVertex(VertexKey const& vertex);
    uint32_t AddVertex(VertexKey const& vertex);

    MeshP GetMesh() { return m_mesh.get(); } //!< The mesh under construction
    double GetTolerance() const { return m_tolerance; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Polyface
{
    DisplayParamsPtr    m_displayParams;
    PolyfaceHeaderPtr   m_polyface;

    Polyface(DisplayParamsR displayParams, PolyfaceHeaderR polyface) : m_displayParams(&displayParams), m_polyface(&polyface) { }

    void Transform(TransformCR transform) { if (m_polyface.IsValid()) m_polyface->Transform(transform); }
    Polyface Clone() const { return Polyface(*m_displayParams, *m_polyface->Clone()); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Strokes
{
    typedef bvector<bvector<DPoint3d>> PointLists;

    DisplayParamsPtr    m_displayParams;
    PointLists          m_strokes;

    Strokes(DisplayParamsR displayParams, PointLists&& strokes) : m_displayParams(&displayParams), m_strokes(std::move(strokes)) { }

    void Transform(TransformCR transform);
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Geometry : RefCountedBase
{
private:
    DisplayParamsPtr        m_params;
    Transform               m_transform;
    DRange3d                m_tileRange;
    DgnElementId            m_entityId;
    mutable size_t          m_facetCount;
    bool                    m_isCurved;
    bool                    m_hasTexture;
protected:
    Geometry(TransformCR tf, DRange3dCR tileRange, DgnElementId entityId, DisplayParamsP params, bool isCurved, DgnDbR db);

    virtual PolyfaceList _GetPolyfaces(IFacetOptionsR facetOptions) = 0;
    virtual StrokesList _GetStrokes (IFacetOptionsR facetOptions) { return StrokesList(); }
    virtual bool _DoDecimate() const { return false; }
    virtual bool _DoVertexCluster() const { return true; }
    virtual size_t _GetFacetCount(FacetCounter& counter) const = 0;
    virtual GeomPartCPtr _GetPart() const { return nullptr; }

    void SetFacetCount(size_t numFacets);
public:
    DisplayParamsPtr GetDisplayParams() const { return m_params; }
    TransformCR GetTransform() const { return m_transform; }
    DRange3dCR GetTileRange() const { return m_tileRange; }
    DgnElementId GetEntityId() const { return m_entityId; } //!< The ID of the element from which this geometry was produced
    size_t GetFacetCount(IFacetOptionsR options) const;
    size_t GetFacetCount(FacetCounter& counter) const { return _GetFacetCount(counter); }
    
    IFacetOptionsPtr CreateFacetOptions(double chordTolerance, NormalMode normalMode) const;

    bool IsCurved() const { return m_isCurved; }
    bool HasTexture() const { return m_hasTexture; }

    PolyfaceList GetPolyfaces(IFacetOptionsR facetOptions) { return _GetPolyfaces(facetOptions); }
    PolyfaceList GetPolyfaces(double chordTolerance, NormalMode normalMode);
    bool DoDecimate() const { return _DoDecimate(); }
    bool DoVertexCluster() const { return _DoVertexCluster(); }
    StrokesList GetStrokes (IFacetOptionsR facetOptions) { return _GetStrokes(facetOptions); }
    GeomPartCPtr GetPart() const { return _GetPart(); }


    //! Create a Geometry for an IGeometry
    static GeometryPtr Create(IGeometryR geometry, TransformCR tf, DRange3dCR tileRange, DgnElementId entityId, DisplayParamsP params, bool isCurved, DgnDbR db);
    //! Create a Geometry for an IBRepEntity
    static GeometryPtr Create(IBRepEntityR solid, TransformCR tf, DRange3dCR tileRange, DgnElementId entityId, DisplayParamsP params, DgnDbR db);
    //! Create a Geometry for text.
    static GeometryPtr Create(TextStringR textString, TransformCR transform, DRange3dCR range, DgnElementId entityId, DisplayParamsP params, DgnDbR db);
    //! Create a Geometry for a part instance.
    static GeometryPtr Create(GeomPartR part, TransformCR transform, DRange3dCR range, DgnElementId entityId, DisplayParamsP params, DgnDbR db);
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct GeomPart : RefCountedBase
{
private:
    DgnGeometryPartId       m_partId;
    DRange3d                m_range;
    GeometryList            m_geometries;
    size_t                  m_instanceCount;
    mutable size_t          m_facetCount;

    virtual uint32_t _GetExcessiveRefCountThreshold() const  override {return 100000;}

protected:
    GeomPart(DgnGeometryPartId partId, DRange3dCR range, GeometryList const& geometry);

public:
    static GeomPartPtr Create(DgnGeometryPartId partId, DRange3dCR range, GeometryList const& geometry) { return new GeomPart(partId, range, geometry); }
    PolyfaceList GetPolyfaces(IFacetOptionsR facetOptions, GeometryCR instance);
    StrokesList GetStrokes(IFacetOptionsR facetOptions, GeometryCR instance);
    size_t GetFacetCount(FacetCounter& counter, GeometryCR instance) const;
    bool IsCurved() const;
    void IncrementInstanceCount() { m_instanceCount++; }
    size_t GetInstanceCount() const { return m_instanceCount; }
    GeometryList const& GetGeometries() const { return m_geometries; }
    DRange3d GetRange() const { return m_range; };
    DgnGeometryPartId GetPartId() const { return m_partId; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct GeometryCollection
{
private:
    MeshList            m_meshes;
    MeshInstanceList    m_instances;
    MeshPartList        m_parts;
public:
    MeshList& Meshes()              { return m_meshes; }
    MeshInstanceList& Instances()   { return m_instances; }
    MeshPartList& Parts()           { return m_parts; }
    bool IsEmpty() const            { return m_meshes.empty() && m_instances.empty(); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Loader : TileTree::TileLoader
{
    DEFINE_T_SUPER(TileTree::TileLoader);

private:
    Loader(TileR tile, TileTree::TileLoadStatePtr loads) : T_Super("", tile, loads"") { }

    virtual folly::Future<BentleyStatus> _GetFromSource() override;
    virtual BentleyStatus _LoadTile() override;
public:
    static LoaderPtr Create(TileR tile, TileTree::TileLoadStatePtr loads) { return new Loader(tile, loads); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Root : TileTree::OctTree::Root
{
    DEFINE_T_SUPER(TileTree::OctTree::Root);
private:
    DgnModelId      m_modelId;
    Utf8String      m_name;
    DRange3d        m_range;
    // ###TODO: Cache (geom parts)

    Root(DgnModelR model, TransformCR transform);

    virtual Utf8CP _GetName() const override { return m_name.c_str(); }
public:
    virtual ~Root() { ClearAllTiles(); }

    DgnModelId GetModelId() const { return m_modelId; }
    GeometricModelPtr GetModel() const { return GetDgnDb().Models().Get<GeometricModel>(GetModelId()); }

    static RootPtr Create(DgnModelR model, TransformCR transform) { return new Root(model, transform); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Tile : TileTree::OctTree::Tile
{
    DEFINE_T_SUPER(TileTree::OctTree::Tile);
private:
    GeometryList    m_geometries;

    Tile(Root& root, TileTree::OctTree::TileId id, Tile const* parent, bool isLeaf);

    virtual TileTree::TileLoaderPtr _CreateTileLoader(TileTree::TileLoadStatePtr) override;
    virtual TileTree::TilePtr _CreateChild(TileTree::OctTree::TileId) const override;
    virtual folly::Future<BentleyStatus> _ReadFromDb() override { return ERROR; }
    virtual folly::Future<BentleyStatus> _SaveToDb() override { return SUCCESS; }
public:
    void ClearGeometry() { m_geometries.clear(); }
    RootCR GetElementRoot() const { return static_cast<RootCR>(GetRoot()); }
    RootR GetElementRoot() { return static_cast<RootR>(GetRootR()); }

    static TilePtr Create(Root& root, TileId id, Tile const* parent, bool isLeaf) { return new Tile(root, id, parent, isLeaf); }
};

END_ELEMENT_TILETREE_NAMESPACE


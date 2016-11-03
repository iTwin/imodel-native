/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/MeshTile.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include "Render.h"
#include "DgnTexture.h"
#include "SolidKernel.h"
#include <map> // NB: Because bmap doesn't support move semantics...

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
class XYZRangeTreeRoot;
END_BENTLEY_GEOMETRY_NAMESPACE

BENTLEY_RENDER_TYPEDEFS(Triangle);
BENTLEY_RENDER_TYPEDEFS(TilePolyline);
BENTLEY_RENDER_TYPEDEFS(TileMesh);
BENTLEY_RENDER_TYPEDEFS(TileMeshBuilder);
BENTLEY_RENDER_TYPEDEFS(TileNode);
BENTLEY_RENDER_TYPEDEFS(ElementTileNode);
BENTLEY_RENDER_TYPEDEFS(ModelTileNode);
BENTLEY_RENDER_TYPEDEFS(TileGenerator);
BENTLEY_RENDER_TYPEDEFS(TileGeometry);
BENTLEY_RENDER_TYPEDEFS(TileDisplayParams);
BENTLEY_RENDER_TYPEDEFS(TileTextureImage);
BENTLEY_RENDER_TYPEDEFS(ITileGenerationFilter);
BENTLEY_RENDER_TYPEDEFS(TileGenerationCache);
BENTLEY_RENDER_TYPEDEFS(ITileGenerationProgressMonitor);

BENTLEY_RENDER_REF_COUNTED_PTR(TileMesh);
BENTLEY_RENDER_REF_COUNTED_PTR(TileNode);
BENTLEY_RENDER_REF_COUNTED_PTR(ElementTileNode);
BENTLEY_RENDER_REF_COUNTED_PTR(ModelTileNode);
BENTLEY_RENDER_REF_COUNTED_PTR(TileMeshBuilder);
BENTLEY_RENDER_REF_COUNTED_PTR(TileGeometry);
BENTLEY_RENDER_REF_COUNTED_PTR(TileTextureImage);
BENTLEY_RENDER_REF_COUNTED_PTR(TileDisplayParams);

BEGIN_BENTLEY_RENDER_NAMESPACE

typedef bvector<TileMeshPtr> TileMeshList;
typedef bvector<TileNodePtr> TileNodeList;
typedef bvector<TileNodeP>   TileNodePList;
typedef bvector<TileGeometryPtr> TileGeometryList;

//=======================================================================================
//! Describes the type of entity from which a tile node or mesh was produced.
//! The IDs of the source entities are recorded in the TileMesh as BeInt64Ids.
// @bsistruct                                                   Paul.Connelly   10/16
//=======================================================================================
enum class TileSource
{
    Element,    //!< Geometry in the tile is associated with DgnElementIds
    Model,      //!< Geometry in the tile is associated with DgnModelIds
    None,       //!< No IDs are associated with geometry in the tile
};

//=======================================================================================
// ! Holds a texture image.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct TileTextureImage : RefCountedBase
    {
    private:
        ImageSource       m_imageSource;

        TileTextureImage(ImageSource&& imageSource) : m_imageSource(std::move(imageSource)) { BeAssert(m_imageSource.IsValid()); }
        TileTextureImage(ImageSource& imageSource) : m_imageSource (imageSource) { BeAssert(m_imageSource.IsValid()); }
    public:
        static TileTextureImagePtr Create(ImageSource&& imageSource) { return new TileTextureImage(std::move(imageSource)); }
        static TileTextureImagePtr Create(ImageSource& imageSource) { return new TileTextureImage(imageSource); }
        static ImageSource Load(TileDisplayParamsCR params, DgnDbR db);

        ImageSourceCR GetImageSource() const { return m_imageSource; }
    };

//=======================================================================================
//! Display params associated with TileGeometry. Based on GraphicParams and GeometryParams.
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct TileDisplayParams : RefCountedBase
{
private:
    uint32_t                        m_fillColor;
    DgnMaterialId                   m_materialId;
    DgnCategoryId                   m_categoryId;
    DgnSubCategoryId                m_subCategoryId;
    mutable TileTextureImagePtr     m_textureImage;
    bool                            m_ignoreLighting;

    TileDisplayParams(GraphicParamsCP graphicParams, GeometryParamsCP geometryParams);
    TileDisplayParams(uint32_t fillColor, TileTextureImageP texture, bool ignoreLighting, DgnCategoryId categoryId, DgnSubCategoryId subCategoryId) 
        : m_fillColor(fillColor), m_textureImage(texture), m_ignoreLighting(ignoreLighting), m_categoryId(categoryId), m_subCategoryId(subCategoryId) { }
    TileDisplayParams (uint32_t fillColor, GeometryParamsCR geometryParams) : 
        m_fillColor (fillColor), m_ignoreLighting(false), m_materialId (geometryParams.GetMaterialId()), m_categoryId(geometryParams.GetCategoryId()), m_subCategoryId(geometryParams.GetSubCategoryId()) {}
public:
    static TileDisplayParamsPtr Create() { return Create(nullptr, nullptr); }
    static TileDisplayParamsPtr Create(GraphicParamsCR graphicParams, GeometryParamsCR geometryParams) { return Create(&graphicParams, &geometryParams); }
    static TileDisplayParamsPtr Create(GraphicParamsCP graphicParams, GeometryParamsCP geometryParams) { return new TileDisplayParams(graphicParams, geometryParams); }
    static TileDisplayParamsPtr Create(uint32_t fillColor, TileTextureImageP textureImage, bool ignoreLighting, DgnCategoryId categoryId = DgnCategoryId(), DgnSubCategoryId subCategoryId = DgnSubCategoryId()) { return new TileDisplayParams(fillColor, textureImage, ignoreLighting, categoryId, subCategoryId); }
    static TileDisplayParamsPtr Create(uint32_t fillColor, GeometryParamsCR geometryParams) { return new TileDisplayParams(fillColor, geometryParams); }

    bool operator<(TileDisplayParams const& rhs) const;

    DgnMaterialId GetMaterialId() const { return m_materialId; }
    uint32_t GetFillColor() const { return m_fillColor; }
    bool GetIgnoreLighting() const { return m_ignoreLighting; }
    DgnTextureCPtr QueryTexture(DgnDbR db) const;
    TileTextureImagePtr& TextureImage() { return m_textureImage; }
    TileTextureImageCP GetTextureImage() const { return m_textureImage.get(); }
    DGNPLATFORM_EXPORT void ResolveTextureImage(DgnDbR db) const;
};

//=======================================================================================
//! Represents one triangle of a TileMesh.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct Triangle
{
    uint32_t    m_indices[3];   // indexes into point/normal/uvparams/elementID vectors
    bool        m_singleSided;

    explicit Triangle(bool singleSided=true) : m_singleSided(singleSided) { SetIndices(0, 0, 0); }
    Triangle(uint32_t indices[3], bool singleSided) : m_singleSided(singleSided) { SetIndices(indices); }

    void SetIndices(uint32_t indices[3]) { SetIndices(indices[0], indices[1], indices[2]); }
    void SetIndices(uint32_t a, uint32_t b, uint32_t c) { m_indices[0] = a; m_indices[1] = b; m_indices[2] = c; }

    bool IsDegenerate() const
        {
        return m_indices[0] == m_indices[1] || m_indices[0] == m_indices[2] || m_indices[1] == m_indices[2];
        }
};

//=======================================================================================
//! Represents a single polyline  of a TileMesh
//! 
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct TilePolyline
{
     bvector <uint32_t>     m_indices;
};  // TilePolyline


//=======================================================================================
//! Represents a single mesh of uniform symbology within a TileNode, consisting of
//! vertex/normal/uv-param/elementID arrays indexed by an array of triangles.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct TileMesh : RefCountedBase
{
private:
    TileDisplayParamsPtr    m_displayParams;
    bvector<Triangle>       m_triangles;
    bvector<TilePolyline>   m_polylines;
    bvector<DPoint3d>       m_points;
    bvector<DVec3d>         m_normals;
    bvector<DPoint2d>       m_uvParams;
    bvector<BeInt64Id>      m_entityIds;   // invalid IDs for clutter geometry
    bool                    m_validIdsPresent;

    explicit TileMesh(TileDisplayParamsPtr& params) : m_displayParams(params), m_validIdsPresent (false) { }

    template<typename T> T const* GetMember(bvector<T> const& from, uint32_t at) const { return at < from.size() ? &from[at] : nullptr; }
public:
    static TileMeshPtr Create(TileDisplayParamsPtr& params) { return new TileMesh(params); }

    DGNPLATFORM_EXPORT DRange3d GetTriangleRange(TriangleCR triangle) const;
    DGNPLATFORM_EXPORT DVec3d GetTriangleNormal(TriangleCR triangle) const;
    DGNPLATFORM_EXPORT bool HasNonPlanarNormals() const;

    TileDisplayParamsCP GetDisplayParams() const { return m_displayParams.get(); } //!< The mesh symbology
    TileDisplayParamsPtr GetDisplayParamsPtr() const { return m_displayParams; } //!< The mesh symbology
    bvector<Triangle> const& Triangles() const { return m_triangles; } //!< Triangles defined as a set of 3 indices into the vertex attribute arrays.
    bvector<TilePolyline> const& Polylines() const { return m_polylines; } //!< Polylines defined as a set of indices into the vertex attribute arrays.
    bvector<DPoint3d> const& Points() const { return m_points; } //!< Position vertex attribute array
    bvector<DVec3d> const& Normals() const { return m_normals; } //!< Normal vertex attribute array
    bvector<DPoint2d> const& Params() const { return m_uvParams; } //!< UV params vertex attribute array
    bvector<BeInt64Id> const& EntityIds() const { return m_entityIds; } //!< Vertex attribute array specifying the ID of the entity (element or model) from which the vertex was produced

    TriangleCP GetTriangle(uint32_t index) const { return GetMember(m_triangles, index); }
    DPoint3dCP GetPoint(uint32_t index) const { return GetMember(m_points, index); }
    DVec3dCP GetNormal(uint32_t index) const { return GetMember(m_normals, index); }
    DPoint2dCP GetParam(uint32_t index) const { return GetMember(m_uvParams, index); }
    BeInt64Id GetEntityId(uint32_t index) const { auto pId = GetMember(m_entityIds, index); return nullptr != pId ? *pId : BeInt64Id(); }
    bool IsEmpty() const { return m_triangles.empty() && m_polylines.empty(); }
    DRange3d GetRange() const { return DRange3d::From (m_points); }

    DRange3d GetUVRange() const { return DRange3d::From (m_uvParams, 0.0); }

    bool ValidIdsPresent() const { return m_validIdsPresent; }

    void AddTriangle(TriangleCR triangle) { m_triangles.push_back(triangle); }
    void AddPolyline (TilePolyline polyline) { m_polylines.push_back(polyline); }
    uint32_t AddVertex(DPoint3dCR point, DVec3dCP normal, DPoint2dCP param, BeInt64Id entityId);
};

//=======================================================================================
//! Builds a single TileMesh to a specified level of detail, optionally applying vertex
//! clustering.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct TileMeshBuilder : RefCountedBase
{
    struct VertexKey
        {
        DPoint3d        m_point;
        DVec3d          m_normal;
        DPoint2d        m_param;
        BeInt64Id       m_entityId;
        bool            m_normalValid = false;
        bool            m_paramValid = false;

        VertexKey() { }
        VertexKey(DPoint3dCR point, DVec3dCP normal, DPoint2dCP param, BeInt64Id entityId) : m_point(point), m_normalValid(nullptr != normal), m_paramValid(nullptr != param), m_entityId(entityId)
            {
            if (m_normalValid) m_normal = *normal;
            if (m_paramValid) m_param = *param;
            }

        DVec3dCP GetNormal() const { return m_normalValid ? &m_normal : nullptr; }
        DPoint2dCP GetParam() const { return m_paramValid ? &m_param : nullptr; }

        struct Comparator
            {
            double  m_tolerance;

            explicit Comparator(double tolerance) : m_tolerance(tolerance) { }
            bool operator()(VertexKey const& lhs, VertexKey const& rhs) const;
            };
        };
private:
    struct TriangleKey
    {
        uint32_t    m_sortedIndices[3];

        TriangleKey() { }
        explicit TriangleKey(TriangleCR triangle);

        bool operator<(TriangleKey const& rhs) const;
    };

    typedef bmap<VertexKey, uint32_t, VertexKey::Comparator> VertexMap;
    typedef bset<TriangleKey> TriangleSet;

    TileMeshPtr             m_mesh;
    VertexMap               m_clusteredVertexMap;
    VertexMap               m_unclusteredVertexMap;
    TriangleSet             m_triangleSet;
    double                  m_tolerance;
    double                  m_areaTolerance;
    size_t                  m_triangleIndex;
    JsonRenderMaterial      m_material;

    TileMeshBuilder(TileDisplayParamsPtr& params, double tolerance, double areaTolerance) : m_mesh(TileMesh::Create(params)), m_unclusteredVertexMap (VertexKey::Comparator(1.0E-4)), m_clusteredVertexMap(VertexKey::Comparator(tolerance)), 
            m_tolerance(tolerance), m_areaTolerance (areaTolerance), m_triangleIndex(0) {  }
public:
    static TileMeshBuilderPtr Create(TileDisplayParamsPtr& params, double tolerance, double areaTolerance) { return new TileMeshBuilder(params, tolerance, areaTolerance); }

    DGNPLATFORM_EXPORT void AddTriangle(PolyfaceVisitorR visitor, DgnMaterialId materialId, DgnDbR dgnDb, BeInt64Id entityId, bool doVertexClustering, bool duplicateTwoSidedTriangles);
    DGNPLATFORM_EXPORT void AddPolyline (bvector<DPoint3d>const& polyline, BeInt64Id entityId, bool doVertexClustering);
    DGNPLATFORM_EXPORT void AddPolyface (PolyfaceQueryCR polyface, DgnMaterialId materialId, DgnDbR dgnDb, BeInt64Id entityId, bool duplicateTwoSidedTriangles);

    void AddTriangle(TriangleCR triangle, TileMeshCR mesh);
    void AddTriangle(TriangleCR triangle);
    uint32_t AddClusteredVertex(VertexKey const& vertex);
    uint32_t AddVertex(VertexKey const& vertex);

    TileMeshP GetMesh() { return m_mesh.get(); } //!< The mesh under construction
    double GetTolerance() const { return m_tolerance; }
};

//=======================================================================================
//! Representation of geometry processed by a TileGenerator.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct TileGeometry : RefCountedBase
{
    enum class NormalMode
        {
        Never,              //!< Never generate normals
        Always,             //!< Always generate normals
        CurvedSurfacesOnly, //!< Generate normals only for curved surfaces
        };

    struct TilePolyface
        {
        TileDisplayParamsPtr    m_displayParams;
        PolyfaceHeaderPtr       m_polyface;

            
        TilePolyface (TileDisplayParamsR displayParams, PolyfaceHeaderPtr& polyface) : m_displayParams (&displayParams), m_polyface (polyface) { }
        };
    typedef bvector<TilePolyface>   T_TilePolyfaces;

private:
    TileDisplayParamsPtr    m_params;
    Transform               m_transform;
    DRange3d                m_tileRange;
    BeInt64Id               m_entityId;
    size_t                  m_facetCount;
    double                  m_facetCountDensity;
    bool                    m_isCurved;
    bool                    m_hasTexture;


protected:
    TileGeometry(TransformCR tf, DRange3dCR tileRange, BeInt64Id entityId, TileDisplayParamsPtr& params, bool isCurved, DgnDbR db);

    virtual T_TilePolyfaces _GetPolyfaces (IFacetOptionsR facetOptions) = 0;
    virtual CurveVectorPtr _GetStrokedCurve(double chordTolerance) = 0;
    virtual bool _IsPolyface() const = 0;

    void SetFacetCount(size_t numFacets);
    IFacetOptionsPtr CreateFacetOptions(double chordTolerance, NormalMode normalMode) const;
public:
    TileDisplayParamsPtr GetDisplayParams() const { return m_params; }
    TransformCR GetTransform() const { return m_transform; }
    DRange3dCR GetTileRange() const { return m_tileRange; }
    BeInt64Id GetEntityId() const { return m_entityId; } //!< The ID of the element from which this geometry was produced

    size_t GetFacetCount() const { return m_facetCount; }
    double GetFacetCountDensity() const { return m_facetCountDensity; }

    bool IsCurved() const { return m_isCurved; }
    bool HasTexture() const { return m_hasTexture; }

    T_TilePolyfaces GetPolyfaces(double chordTolerance, NormalMode normalMode);
    bool IsPolyface() const { return _IsPolyface(); }
    CurveVectorPtr    GetStrokedCurve (double chordTolerance) { return _GetStrokedCurve(chordTolerance); }
    
    //! Create a TileGeometry for an IGeometry
    static TileGeometryPtr Create(IGeometryR geometry, TransformCR tf, DRange3dCR tileRange, BeInt64Id entityId, TileDisplayParamsPtr& params, IFacetOptionsR facetOptions, bool isCurved, DgnDbR db);
    //! Create a TileGeometry for an IBRepEntity
    static TileGeometryPtr Create(IBRepEntityR solid, TransformCR tf, DRange3dCR tileRange, BeInt64Id entityId, TileDisplayParamsPtr& params, IFacetOptionsR facetOptions, DgnDbR db);
};

//=======================================================================================
//! Filters elements during TileNode generation. Elements are selected according to their
//! intersection with a TileNode's range, then tested against the supplied ITileGenerationFilter
//! to apply additional selection criteria.
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct ITileGenerationFilter
{
protected:
    virtual bool _AcceptElement(DgnElementId elementId) = 0;
public:
    //! Invoked for each element in the tile's range. Returns false to exclude the element from the tile geometry, or true to include it.
    bool AcceptElement(DgnElementId elementId) { return _AcceptElement(elementId); }
};

//=======================================================================================
//! Accepts all elements.
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct UnconditionalTileGenerationFilter : ITileGenerationFilter
{
protected:
    virtual bool _AcceptElement(DgnElementId) override { return true; }
};

//=======================================================================================
//! Filters elements according to a set of models and categories.
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct TileModelCategoryFilter : ITileGenerationFilter
{
    struct ModelAndCategorySet : BeSQLite::VirtualSet
        {
    private:
        DgnModelIdSet       m_models;
        DgnCategoryIdSet    m_categories;

        virtual bool _IsInSet(int nVals, BeSQLite::DbValue const* vals) const override
            {
            return m_models.Contains(DgnModelId(vals[0].GetValueUInt64())) && m_categories.Contains(DgnCategoryId(vals[1].GetValueUInt64()));
            }
    public:
        ModelAndCategorySet(DgnModelIdSet const* models, DgnCategoryIdSet const* categories)
            {
            if (nullptr != models)      m_models = *models;
            if (nullptr != categories)  m_categories = *categories;
            }

        bool IsEmpty() const { return m_models.empty() && m_categories.empty(); }
        };
protected:
    ModelAndCategorySet                     m_set;
    BeSQLite::EC::CachedECSqlStatementPtr   m_stmt;

    DGNPLATFORM_EXPORT virtual bool _AcceptElement(DgnElementId elementId) override;
public:
    DGNPLATFORM_EXPORT TileModelCategoryFilter(DgnDbR dgndb, DgnModelIdSet const* modelIds, DgnCategoryIdSet const* categoryIds);

    bool IsEmpty() const { return m_set.IsEmpty(); }
};


//=======================================================================================
//! Filters elements according to the viewed models and categories associated with a
//! ViewController.
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct TileViewControllerFilter : TileModelCategoryFilter
{
public:
    TileViewControllerFilter(SpatialViewControllerCR view) : TileModelCategoryFilter(view.GetDgnDb(), &view.GetViewedModels(), &view.GetViewedCategories()) { }

};

//=======================================================================================
//! Caches information used during tile generation.
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct TileGenerationCache
{
    // ###TODO: Put upper limit on sizes of geometry source/list caches...
    // The following options are mutually exclusive
    enum class Options
    {
        None = 0,               // cache nothin
        CacheGeometrySources,   // cache GeometrySources by element ID
        CacheGeometryLists,     // cache TileGeometryLists by element ID
    };
private:
    typedef bmap<DgnElementId, TileGeometryList>                    GeometryListMap;
    typedef std::map<DgnElementId, std::unique_ptr<GeometrySource>> GeometrySourceMap;

    XYZRangeTreeRoot*           m_tree;
    mutable GeometryListMap     m_geometry;
    mutable GeometrySourceMap   m_geometrySources;
    mutable BeMutex             m_mutex;    // for geometry cache
    mutable BeSQLite::BeDbMutex m_dbMutex;  // for multi-threaded access to database
    Options                     m_options;

    friend struct TileGenerator; // Invokes Populate() from ctor
    TileGenerationCache(Options options = Options::CacheGeometrySources);
    void Populate(DgnDbR db, DgnModelId modelId, ITileGenerationFilterP filter);
public:
    DGNPLATFORM_EXPORT ~TileGenerationCache();

    XYZRangeTreeRoot& GetTree() const { return *m_tree; }
    DGNPLATFORM_EXPORT DRange3d GetRange() const;

    bool WantCacheGeometrySources() const { return Options::CacheGeometrySources == m_options; }
    GeometrySourceCP GetCachedGeometrySource(DgnElementId elementId) const;
    GeometrySourceCP AddCachedGeometrySource(std::unique_ptr<GeometrySource>& source, DgnElementId elementId) const;

    bool WantCacheGeometry() const { return Options::CacheGeometryLists == m_options; }
    bool GetCachedGeometry(TileGeometryList& geometry, DgnElementId elementId) const;
    void AddCachedGeometry(DgnElementId elementId, TileGeometryList&& geometry) const;

    BeSQLite::BeDbMutex& GetDbMutex() const { return m_dbMutex; }
};

//=======================================================================================
//! Represents one tile in a HLOD tree occupying a given range and containing higher-LOD
//! child tiles within the same range.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct TileNode : RefCountedBase
{
protected:
    DRange3d            m_dgnRange;
    TileNodeList        m_children;
    size_t              m_depth;
    size_t              m_siblingIndex;
    double              m_tolerance;
    TileNodeP           m_parent;
    Transform           m_transformFromDgn;
    mutable DRange3d    m_publishedRange;
    DgnModelCPtr        m_model;
    bool                m_isEmpty;

    TileNode(DgnModelCR model, TransformCR transformFromDgn) : TileNode(model, DRange3d::NullRange(), transformFromDgn, 0, 0, nullptr) { }
    TileNode(DgnModelCR model, DRange3dCR range, TransformCR transformFromDgn, size_t depth, size_t siblingIndex, TileNodeP parent, double tolerance = 0.0)
        : m_dgnRange(range), m_depth(depth), m_siblingIndex(siblingIndex), m_tolerance(tolerance), m_parent(parent), m_transformFromDgn(transformFromDgn), m_publishedRange(DRange3d::NullRange()), m_model(&model), m_isEmpty(true) { }

    TransformCR GetTransformFromDgn() const { return m_transformFromDgn; }


    virtual TileSource _GetSource() const = 0;
    virtual TileMeshList _GenerateMeshes(DgnDbR dgndb, TileGeometry::NormalMode normalMode=TileGeometry::NormalMode::CurvedSurfacesOnly, bool twoSidedTriangles=false, bool doPolylines=false) const = 0;
    virtual void _CollectGeometry(TileGenerationCacheCR cache, DgnDbR db, bool* leafThresholdExceeded, double tolerance, size_t leafCountThreshold) { }
    virtual void _ClearGeometry() { }

public:
    DgnModelCR GetModel() const { return *m_model; }
    DRange3dCR GetDgnRange() const { return m_dgnRange; }
    DRange3d GetTileRange() const { DRange3d range = m_dgnRange; m_transformFromDgn.Multiply(range, range); return range; }
    DPoint3d GetTileCenter() const { DRange3d range = GetTileRange(); return DPoint3d::FromInterpolate (range.low, .5, range.high); }
    size_t GetDepth() const { return m_depth; } //!< This node's depth from the root tile node
    size_t GetSiblingIndex() const { return m_siblingIndex; } //!< This node's order within its siblings at the same depth
    double GetTolerance() const { return m_tolerance; }

    TileNodeCP GetParent() const { return m_parent; } //!< The direct parent of this node
    TileNodeP GetParent() { return m_parent; } //!< The direct parent of this node
    TileNodeList const& GetChildren() const { return m_children; } //!< The direct children of this node
    TileNodeList& GetChildren() { return m_children; } //!< The direct children of this node
    void SetDgnRange (DRange3dCR range) { m_dgnRange = range; }
    void SetTileRange(DRange3dCR range) { Transform tf; DRange3d dgnRange = range; tf.InverseOf(m_transformFromDgn); tf.Multiply(dgnRange, dgnRange); SetDgnRange(dgnRange); }
    void SetPublishedRange (DRange3dCR publishedRange) const { m_publishedRange = publishedRange; }
    DRange3dCR GetPublishedRange() const { return m_publishedRange; }


    DGNPLATFORM_EXPORT size_t GetNodeCount() const;
    DGNPLATFORM_EXPORT size_t GetMaxDepth() const;
    DGNPLATFORM_EXPORT void GetTiles(TileNodePList& tiles);
    DGNPLATFORM_EXPORT TileNodePList GetTiles();
    DGNPLATFORM_EXPORT WString GetNameSuffix() const;
    DGNPLATFORM_EXPORT size_t   GetNameSuffixId() const;
    DGNPLATFORM_EXPORT WString GetFileName (WCharCP rootName, WCharCP extension) const;
    void SetIsEmpty(bool isEmpty) { m_isEmpty = isEmpty; }
    bool GetIsEmpty () const { return m_isEmpty; }

    void  CollectGeometry(TileGenerationCacheCR cache, DgnDbR db, bool* leafThresholdExceeded, double tolerance, size_t leafCountThreshold) { _CollectGeometry(cache, db, leafThresholdExceeded, tolerance, leafCountThreshold); }
    void  ClearGeometry() { _ClearGeometry(); }
    TileSource GetSource() const { return _GetSource(); }
    TileMeshList GenerateMeshes(DgnDbR dgndb, TileGeometry::NormalMode normalMode=TileGeometry::NormalMode::CurvedSurfacesOnly, bool twoSidedTriangles=false, bool doPolylines=false) const
        { return _GenerateMeshes(dgndb, normalMode, twoSidedTriangles, doPolylines); }
};

//=======================================================================================
//! A TileNode generated from a set of elements.
// @bsistruct                                                   Paul.Connelly   10/16
//=======================================================================================
struct ElementTileNode : TileNode
{
private:
    bool                    m_isLeaf;
    TileGeometryList        m_geometries;

protected:
    ElementTileNode(DgnModelCR model, TransformCR transformFromDgn) : TileNode(model, transformFromDgn), m_isLeaf(false) { }
    ElementTileNode(DgnModelCR model, DRange3dCR range, TransformCR transformFromDgn, size_t depth, size_t siblingIndex, TileNodeP parent, double tolerance = 0.0)
        : TileNode(model, range, transformFromDgn, depth, siblingIndex, parent, tolerance), m_isLeaf(false) { }


    DGNPLATFORM_EXPORT virtual TileMeshList _GenerateMeshes(DgnDbR, TileGeometry::NormalMode, bool, bool) const override;
    virtual TileSource _GetSource() const override final { return TileSource::Element; }
    virtual void _CollectGeometry(TileGenerationCacheCR cache, DgnDbR db, bool* leafThresholdExceeded, double tolerance, size_t leafCountThreshold) override;
    virtual void _ClearGeometry() override { m_geometries.clear(); }
public:
    static ElementTileNodePtr Create(DgnModelCR model, TransformCR transformFromDgn) { return new ElementTileNode(model, transformFromDgn); }
    static ElementTileNodePtr Create(DgnModelCR model, DRange3dCR dgnRange, TransformCR transformFromDgn, size_t depth, size_t siblingIndex, TileNodeP parent)
        { return new ElementTileNode(model, dgnRange, transformFromDgn, depth, siblingIndex, parent); }

    void SetTolerance(double tolerance) { m_tolerance = tolerance; }
    void SetIsLeaf(bool isLeaf) { m_isLeaf = isLeaf; }
    TileGeometryList const& GetGeometries() const { return m_geometries; }

    DGNPLATFORM_EXPORT static void ComputeChildTileRanges(bvector<DRange3d>& subTileRanges, DRange3dCR range, size_t splitCount);
};

//=======================================================================================
//! A TileNode generated from a model implementing IGenerateMeshTiles.
//! This does not include geometry of elements within the model.
// @bsistruct                                                   Paul.Connelly   10/16
//=======================================================================================
struct ModelTileNode : TileNode
{
protected:
    ModelTileNode(DgnModelCR model, TransformCR transformFromDgn) : TileNode(model, transformFromDgn) { }
    ModelTileNode(DgnModelCR model, DRange3dCR range, TransformCR transformFromDgn, size_t depth, size_t siblingIndex, TileNodeP parent, double tolerance = 0.0)
        : TileNode(model, range, transformFromDgn, depth, siblingIndex, parent, tolerance) { m_isEmpty = false; }

    virtual TileSource _GetSource() const override final { return TileSource::Model; }
};

//=======================================================================================
//! Interface adopted by an object which tracks progress of the tile generation process
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ITileGenerationProgressMonitor
{
    virtual void _IndicateProgress(uint32_t completed, uint32_t total) { } //!< Invoked to announce the current ratio completed
    virtual bool _WasAborted() { return false; } //!< Return true to abort tile generation
};

//=======================================================================================
//! Generates a HLOD tree of TileNodes from a set of tiles.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct TileGenerator
{
    enum class Status
    {
        Success = SUCCESS,
        NoGeometry,
        Aborted,
    };

    //! Interface adopted by an object which collects generated tiles
    struct EXPORT_VTABLE_ATTRIBUTE ITileCollector
    {
        //! Invoked from one of several worker threads for each generated tile.
        virtual Status _AcceptTile(TileNodeCR tileNode) = 0;
        //! Invoked before a model is processed.
        virtual Status _BeginProcessModel(DgnModelCR model) { return Status::Success; }
        //! Invoked after a model is processed, with the result of processing.
        virtual Status _EndProcessModel(DgnModelCR model, TileNodeP rootTile, Status status) { return status; }
    };

    //! Accumulates statistics during tile generation
    struct Statistics
    {
        size_t      m_tileCount = 0;
        size_t      m_tileDepth = 0;
        double      m_collectionTime = 0.0;
        double      m_tileCreationTime = 0.0;
        double      m_cachePopulationTime = 0.0;
    };
private:
    Statistics                      m_statistics;
    ITileGenerationProgressMonitorR m_progressMeter;
    Transform                       m_transformFromDgn;
    DgnDbR                          m_dgndb;
    double                          m_totalVolume;
    double                          m_completedVolume;
    BeAtomic<uint32_t>              m_totalTiles;
    BeAtomic<uint32_t>              m_completedTiles;

    void    ProcessTile(ElementTileNodeR tile, ITileCollector& collector, double leafTolerance, size_t maxPointsPerTile, TileGenerationCacheCR generationCache);
    Status GenerateElementTiles(TileNodePtr& root, ITileCollector& collector, double leafTolerance, size_t maxPointsPerTile, DgnModelR model);
public:
    DGNPLATFORM_EXPORT explicit TileGenerator(TransformCR transformFromDgn, DgnDbR dgndb, ITileGenerationFilterP filter=nullptr, ITileGenerationProgressMonitorP progress=nullptr);

    DgnDbR GetDgnDb() const { return m_dgndb; }
    TransformCR GetTransformFromDgn() const { return m_transformFromDgn; }
    Statistics const& GetStatistics() const { return m_statistics; }
    ITileGenerationProgressMonitorR GetProgressMeter () { return m_progressMeter; }

    DGNPLATFORM_EXPORT Status GenerateTiles(TileNodePtr& root, ITileCollector& collector, double leafTolerance, size_t maxPointsPerTile, DgnModelId modelId);
};

//=======================================================================================
// Interface for models to generate HLOD tree of TileNodes 
// @bsistruct                                                   Ray.Bentley     08/2016
//=======================================================================================
struct IGenerateMeshTiles
{
    virtual TileGenerator::Status _GenerateMeshTiles(TileNodePtr& rootTile, TransformCR transformDbToTile, TileGenerator::ITileCollector& collector, ITileGenerationProgressMonitorR progressMeter) = 0;

};  // IPublishModelMeshTiles

END_BENTLEY_RENDER_NAMESPACE


/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/MeshTile.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include "Render.h"
#include "DgnTexture.h"
#include "SolidKernel.h"
#include <map> // NB: Because bmap doesn't support move semantics...
#include <folly/futures/Future.h>

#include <DgnPlatform/DgnPlatformLib.h>

BENTLEY_RENDER_TYPEDEFS(TileTriangle);
BENTLEY_RENDER_TYPEDEFS(TilePolyline);
BENTLEY_RENDER_TYPEDEFS(TileMesh);
BENTLEY_RENDER_TYPEDEFS(TileMeshPart);
BENTLEY_RENDER_TYPEDEFS(TileMeshPointCloud);
BENTLEY_RENDER_TYPEDEFS(TileMeshInstance);
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
BENTLEY_RENDER_TYPEDEFS(TileModelDelta);
BENTLEY_RENDER_TYPEDEFS(TileGeomPart);
BENTLEY_RENDER_TYPEDEFS(PublishableTileGeometry);

BENTLEY_RENDER_REF_COUNTED_PTR(TileMesh);
BENTLEY_RENDER_REF_COUNTED_PTR(TileMeshPart);
BENTLEY_RENDER_REF_COUNTED_PTR(TileMeshPointCloud);
BENTLEY_RENDER_REF_COUNTED_PTR(TileNode);
BENTLEY_RENDER_REF_COUNTED_PTR(ElementTileNode);
BENTLEY_RENDER_REF_COUNTED_PTR(ModelTileNode);
BENTLEY_RENDER_REF_COUNTED_PTR(TileMeshBuilder);
BENTLEY_RENDER_REF_COUNTED_PTR(TileGeometry);
BENTLEY_RENDER_REF_COUNTED_PTR(TileTextureImage);
BENTLEY_RENDER_REF_COUNTED_PTR(TileDisplayParams);
BENTLEY_RENDER_REF_COUNTED_PTR(TileGenerationCache);
BENTLEY_RENDER_REF_COUNTED_PTR(TileModelDelta);
BENTLEY_RENDER_REF_COUNTED_PTR(TileGeomPart);

BEGIN_BENTLEY_RENDER_NAMESPACE

typedef bvector<TileMeshPtr>            TileMeshList;
typedef bvector<TileMeshInstance>       TileMeshInstanceList;
typedef bvector<TileMeshPartPtr>        TileMeshPartList;
typedef bvector<TileMeshPointCloudPtr>  TileMeshPointCloudList;
typedef bvector<TileNodePtr>            TileNodeList;
typedef bvector<TileNodeP>              TileNodePList;
typedef bvector<TileGeometryPtr>        TileGeometryList;

//=======================================================================================
//! Enumerates possible results of tile generation.
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
enum class TileGeneratorStatus
{
    Success = SUCCESS,  //!< Operation successful
    NoGeometry,         //!< Operation produced no geometry
    Aborted,            //!< Operation was aborted - perhaps by user, or caller, or because contents of model changed
};

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
    DgnCategoryId                   m_categoryId;
    DgnSubCategoryId                m_subCategoryId;
    uint32_t                        m_fillColor;
    uint32_t                        m_rasterWidth;
    DgnMaterialId                   m_materialId;
    mutable TileTextureImagePtr     m_textureImage;
    bool                            m_ignoreLighting;

    DGNPLATFORM_EXPORT TileDisplayParams(GraphicParamsCP graphicParams, GeometryParamsCP geometryParams, bool ignoreLighting);
    TileDisplayParams(uint32_t fillColor, TileTextureImageP texture, bool ignoreLighting) : m_fillColor(fillColor), m_textureImage(texture), m_ignoreLighting(ignoreLighting), m_rasterWidth(0) { }
    TileDisplayParams(uint32_t fillColor, GeometryParamsCR geometryParams) : m_fillColor(fillColor), m_ignoreLighting(false), m_materialId(geometryParams.GetMaterialId()), m_rasterWidth(0) {}
    TileDisplayParams(uint32_t fillColor, DgnMaterialId materialId) : m_fillColor(fillColor), m_materialId(materialId), m_ignoreLighting(false), m_rasterWidth(0) {}
public:
    static TileDisplayParamsPtr Create() { return Create(nullptr, nullptr); }
    static TileDisplayParamsPtr Create(GraphicParamsCR graphicParams, GeometryParamsCR geometryParams, bool ignoreLighting = false) { return Create(&graphicParams, &geometryParams, ignoreLighting); }
    static TileDisplayParamsPtr Create(GraphicParamsCP graphicParams, GeometryParamsCP geometryParams, bool ignoreLighting = false) { return new TileDisplayParams(graphicParams, geometryParams, ignoreLighting); }
    static TileDisplayParamsPtr Create(uint32_t fillColor, TileTextureImageP textureImage, bool ignoreLighting) { return new TileDisplayParams(fillColor, textureImage, ignoreLighting); }
    static TileDisplayParamsPtr Create(uint32_t fillColor, GeometryParamsCR geometryParams) { return new TileDisplayParams(fillColor, geometryParams); }
    static TileDisplayParamsPtr Create(uint32_t fillColor, DgnMaterialId materialId) { return new TileDisplayParams(fillColor, materialId); }

    DGNPLATFORM_EXPORT bool operator<(TileDisplayParams const& rhs) const;

    DgnCategoryId GetCategoryId() const { return m_categoryId; }
    DgnSubCategoryId GetSubCategoryId() const { return m_subCategoryId; }
    DgnMaterialId GetMaterialId() const { return m_materialId; }
    uint32_t GetFillColor() const { return m_fillColor; }
    uint32_t GetRasterWidth() const { return m_rasterWidth; }
    bool GetIgnoreLighting() const { return m_ignoreLighting; }
    DgnTextureCPtr QueryTexture(DgnDbR db) const;
    TileTextureImagePtr& TextureImage() { return m_textureImage; }
    TileTextureImageCP GetTextureImage() const { return m_textureImage.get(); }
    DGNPLATFORM_EXPORT void ResolveTextureImage(DgnDbR db) const;
};

//=======================================================================================
//! Represents a mesh instance.
// @bsistruct                                                   Ray.Bentley     12/2016
//=======================================================================================
struct TileMeshInstance
{
private:
    DgnElementId        m_instanceId;
    Transform           m_transform;

public:
    DgnElementId GetId() const { return m_instanceId; }
    TransformCR GetTransform() const { return m_transform; }
    TileMeshInstance (DgnElementId instanceId , TransformCR transform) : m_instanceId(instanceId), m_transform(transform) { }

};  // TileMeshInstance

//=======================================================================================
//! Represents a mesh part.
// @bsistruct                                                   Ray.Bentley     12/2016
//=======================================================================================
struct TileMeshPart : RefCountedBase
{
private:
    TileMeshList            m_meshes;
    TileMeshInstanceList    m_instances;

    TileMeshPart (TileMeshList&& meshes) : m_meshes(meshes) { }

    virtual uint32_t _GetExcessiveRefCountThreshold() const  override {return 100000;} \

public:
    TileMeshList const& Meshes() const { return m_meshes; }
    TileMeshInstanceList const& Instances() const { return m_instances; }
    void AddInstance (TileMeshInstanceCR instance) { m_instances.push_back(instance); }

    static TileMeshPartPtr Create (TileMeshList&& meshes) { return new TileMeshPart(std::move(meshes)); }

};  // TileMeshPart.

//=======================================================================================
//! Represents one triangle of a TileMesh.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct TileTriangle
{
    uint32_t    m_indices[3];   // indexes into point/normal/uvparams/elementID vectors
    bool        m_singleSided;

    explicit TileTriangle(bool singleSided=true) : m_singleSided(singleSided) { SetIndices(0, 0, 0); }
    TileTriangle(uint32_t indices[3], bool singleSided) : m_singleSided(singleSided) { SetIndices(indices); }
    TileTriangle(uint32_t a, uint32_t b, uint32_t c, bool singleSided) : m_singleSided(singleSided) { SetIndices(a, b, c); }

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

public:
    bvector<uint32_t> const& GetIndices() const { return m_indices; }
    void AddIndex(uint32_t index)  { if(m_indices.empty() || m_indices.back() != index) m_indices.push_back(index); }
    void Clear() { m_indices.clear(); }
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
    bvector<TileTriangle>   m_triangles;
    bvector<TilePolyline>   m_polylines;
    bvector<DPoint3d>       m_points;
    bvector<DVec3d>         m_normals;
    bvector<DPoint2d>       m_uvParams;
    bvector<DgnElementId>   m_entityIds;
    bool                    m_validIdsPresent;

    explicit TileMesh(TileDisplayParamsPtr& params) : m_displayParams(params), m_validIdsPresent(false) { }
    template<typename T> T const* GetMember(bvector<T> const& from, uint32_t at) const { return at < from.size() ? &from[at] : nullptr; }
public:
    static TileMeshPtr Create(TileDisplayParamsPtr& params) { return new TileMesh(params); }

    DGNPLATFORM_EXPORT DRange3d GetTriangleRange(TileTriangleCR triangle) const;
    DGNPLATFORM_EXPORT DVec3d GetTriangleNormal(TileTriangleCR triangle) const;
    DGNPLATFORM_EXPORT bool HasNonPlanarNormals() const;
    DGNPLATFORM_EXPORT bool RemoveEntityGeometry(bset<DgnElementId> const& ids);

    TileDisplayParamsCP GetDisplayParams() const { return m_displayParams.get(); } //!< The mesh symbology
    TileDisplayParamsPtr GetDisplayParamsPtr() const { return m_displayParams; } //!< The mesh symbology
    bvector<TileTriangle> const& Triangles() const { return m_triangles; } //!< TileTriangles defined as a set of 3 indices into the vertex attribute arrays.
    bvector<TilePolyline> const& Polylines() const { return m_polylines; } //!< Polylines defined as a set of indices into the vertex attribute arrays.
    bvector<DPoint3d> const& Points() const { return m_points; } //!< Position vertex attribute array
    bvector<DVec3d> const& Normals() const { return m_normals; } //!< Normal vertex attribute array
    bvector<DPoint2d> const& Params() const { return m_uvParams; } //!< UV params vertex attribute array
    bvector<DgnElementId> const& EntityIds() const { return m_entityIds; } //!< Vertex attribute array specifying the ID of the entity(element or model) from which the vertex was produced
    bvector<DPoint3d>& PointsR() { return m_points; } //!< Position vertex attribute array
    bvector<DVec3d>& NormalsR() { return m_normals; } //!< Normal vertex attribute array
    bvector<DPoint2d>& ParamsR() { return m_uvParams; } //!< UV params vertex attribute array
    bvector<DgnElementId>& EntityIdsR() { return m_entityIds; }

    TileTriangleCP GetTriangle(uint32_t index) const { return GetMember(m_triangles, index); }
    DPoint3dCP GetPoint(uint32_t index) const { return GetMember(m_points, index); }
    DVec3dCP GetNormal(uint32_t index) const { return GetMember(m_normals, index); }
    DPoint2dCP GetParam(uint32_t index) const { return GetMember(m_uvParams, index); }
    DgnElementId GetEntityId(uint32_t index) const { auto pId = GetMember(m_entityIds, index); return nullptr != pId ? *pId : DgnElementId(); }
    DgnElementId GetTriangleEntityId(TileTriangleCR triangle) const { return GetEntityId(triangle.m_indices[0]); }
    DgnElementId GetPolylineEntityId(TilePolylineCR polyline) const { return polyline.GetIndices().empty() ? DgnElementId() : GetEntityId(polyline.GetIndices().front()); }
    bool IsEmpty() const { return m_triangles.empty() && m_polylines.empty(); }
    DRange3d GetRange() const { return DRange3d::From(m_points); }
    DRange3d GetUVRange() const { return DRange3d::From(m_uvParams, 0.0); }

    bool ValidIdsPresent() const { return m_validIdsPresent; }

    void AddTriangle(TileTriangleCR triangle) { m_triangles.push_back(triangle); }
    void AddPolyline(TilePolyline polyline) { m_polylines.push_back(polyline); }
    
    DGNPLATFORM_EXPORT void AddMesh(TileMeshCR mesh);
    uint32_t AddVertex(DPoint3dCR point, DVec3dCP normal, DPoint2dCP param, DgnElementId entityId);
    DGNPLATFORM_EXPORT void SetValidIdsPresent(bool validIdsPresent) { m_validIdsPresent = validIdsPresent; }
};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     06/2016
+===============+===============+===============+===============+===============+======*/
struct TileMeshMergeKey
{
    TileDisplayParamsCP m_params;                                                                                                                                                     
    bool                m_hasNormals;
    bool                m_hasFacets;

    TileMeshMergeKey() : m_params(nullptr), m_hasNormals(false), m_hasFacets(false) { }
    TileMeshMergeKey(TileDisplayParamsCR params, bool hasNormals, bool hasFacets) : m_params(&params), m_hasNormals(hasNormals), m_hasFacets(hasFacets) { }
    TileMeshMergeKey(TileMeshCR mesh) : TileMeshMergeKey(*mesh.GetDisplayParams(),  !mesh.Normals().empty(), !mesh.Triangles().empty()) { }

    bool operator<(TileMeshMergeKey const& rhs) const
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

        struct Comparator
            {
            double  m_tolerance;

            explicit Comparator(double tolerance) : m_tolerance(tolerance) { }
            bool operator()(VertexKey const& lhs, VertexKey const& rhs) const;
            };
        };
private:
    struct TileTriangleKey
    {
        uint32_t    m_sortedIndices[3]; 

        TileTriangleKey() { }
        explicit TileTriangleKey(TileTriangleCR triangle);

        bool operator<(TileTriangleKey const& rhs) const;
    };

    typedef bmap<VertexKey, uint32_t, VertexKey::Comparator> VertexMap;
    typedef bset<TileTriangleKey> TileTriangleSet;

    TileMeshPtr             m_mesh;
    VertexMap               m_clusteredVertexMap;
    VertexMap               m_unclusteredVertexMap;
    TileTriangleSet         m_triangleSet;
    double                  m_tolerance;
    double                  m_areaTolerance;
    size_t                  m_triangleIndex;
    JsonRenderMaterial      m_material;

    TileMeshBuilder(TileDisplayParamsPtr& params, double tolerance, double areaTolerance) : m_mesh(TileMesh::Create(params)), m_unclusteredVertexMap(VertexKey::Comparator(1.0E-4)), m_clusteredVertexMap(VertexKey::Comparator(tolerance)), 
            m_tolerance(tolerance), m_areaTolerance(areaTolerance), m_triangleIndex(0) {  }
public:
    static TileMeshBuilderPtr Create(TileDisplayParamsPtr& params, double tolerance, double areaTolerance) { return new TileMeshBuilder(params, tolerance, areaTolerance); }

    DGNPLATFORM_EXPORT void AddTriangle(PolyfaceVisitorR visitor, DgnMaterialId materialId, DgnDbR dgnDb, DgnElementId entityId, bool doVertexClustering, bool duplicateTwoSidedTileTriangles, bool includeParams);
    DGNPLATFORM_EXPORT void AddPolyline(bvector<DPoint3d>const& polyline, DgnElementId entityId, bool doVertexClustering);
    DGNPLATFORM_EXPORT void AddPolyface(PolyfaceQueryCR polyface, DgnMaterialId materialId, DgnDbR dgnDb, DgnElementId entityId, bool duplicateTwoSidedTileTriangles, bool includeParams);

    void AddMesh(TileTriangleCR triangle);
    void AddTriangle(TileTriangleCR triangle);
    uint32_t AddClusteredVertex(VertexKey const& vertex);
    uint32_t AddVertex(VertexKey const& vertex);

    TileMeshP GetMesh() { return m_mesh.get(); } //!< The mesh under construction
    double GetTolerance() const { return m_tolerance; }
};

//=======================================================================================
//! Represents a point cloud within a tile node.
// @bsistruct                                                   Ray.Bentley     01/2017
//=======================================================================================
struct TileMeshPointCloud : RefCountedBase
{

private:
    TileDisplayParamsPtr    m_displayParams;
    bvector<DPoint3d>       m_points;
    
    TileMeshPointCloud (TileDisplayParamsPtr& params, DPoint3dCP points, size_t nPoints, TransformCR transform, double clusterTolerance);

public:
    bvector<DPoint3d> const& Points() { return m_points; }
    DGNPLATFORM_EXPORT static TileMeshPointCloudPtr Create(TileDisplayParamsPtr& params, DPoint3dCP points, size_t nPoints, TransformCR transform, double clusterTolerance) { return new TileMeshPointCloud (params, points,  nPoints, transform, clusterTolerance); }

};  // TilePointCloud

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
            
        TilePolyface(TileDisplayParamsR displayParams, PolyfaceHeaderR polyface) : m_displayParams(&displayParams), m_polyface(&polyface) { }
        void Transform(TransformCR transform) { if (m_polyface.IsValid()) m_polyface->Transform (transform); }
        TilePolyface    Clone() const { return TilePolyface(*m_displayParams, *m_polyface->Clone()); }
        };
    struct TileStrokes
        {
        TileDisplayParamsPtr        m_displayParams;
        bvector<bvector<DPoint3d>>  m_strokes;

        void Transform(TransformCR transform);

        TileStrokes (TileDisplayParamsR displayParams, bvector<bvector<DPoint3d>>&& strokes) : m_displayParams(&displayParams),  m_strokes(std::move(strokes)) { }
        }; 

    typedef bvector<TilePolyface>   T_TilePolyfaces;
    typedef bvector<TileStrokes>    T_TileStrokes;



private:
    TileDisplayParamsPtr    m_params;
    Transform               m_transform;
    DRange3d                m_tileRange;
    DgnElementId            m_entityId;
    bool                    m_isCurved;
    bool                    m_hasTexture;
    mutable size_t          m_facetCount;


protected:
    TileGeometry(TransformCR tf, DRange3dCR tileRange, DgnElementId entityId, TileDisplayParamsPtr& params, bool isCurved, DgnDbR db);

    virtual T_TilePolyfaces _GetPolyfaces(IFacetOptionsR facetOptions) = 0;
    virtual T_TileStrokes _GetStrokes (IFacetOptionsR facetOptions) { return T_TileStrokes(); }
    virtual bool _DoDecimate() const { return false; }
    virtual bool _DoVertexCluster() const { return true; }
    virtual size_t _GetFacetCount(FacetCounter& counter) const = 0;
    virtual TileGeomPartCPtr _GetPart() const { return TileGeomPartCPtr(); }

    void SetFacetCount(size_t numFacets);
public:
    TileDisplayParamsPtr GetDisplayParams() const { return m_params; }
    TransformCR GetTransform() const { return m_transform; }
    DRange3dCR GetTileRange() const { return m_tileRange; }
    DgnElementId GetEntityId() const { return m_entityId; } //!< The ID of the element from which this geometry was produced
    size_t GetFacetCount(IFacetOptionsR options) const;
    size_t GetFacetCount(FacetCounter& counter) const { return _GetFacetCount(counter); }
    
    IFacetOptionsPtr CreateFacetOptions(double chordTolerance, NormalMode normalMode) const;

    bool IsCurved() const { return m_isCurved; }
    bool HasTexture() const { return m_hasTexture; }

    T_TilePolyfaces GetPolyfaces(IFacetOptionsR facetOptions) { return _GetPolyfaces(facetOptions); }
    T_TilePolyfaces GetPolyfaces(double chordTolerance, NormalMode normalMode);
    bool DoDecimate() const { return _DoDecimate(); }
    bool DoVertexCluster() const { return _DoVertexCluster(); }
    T_TileStrokes GetStrokes (IFacetOptionsR facetOptions) { return _GetStrokes(facetOptions); }
    TileGeomPartCPtr GetPart() const { return _GetPart(); }


    //! Create a TileGeometry for an IGeometry
    static TileGeometryPtr Create(IGeometryR geometry, TransformCR tf, DRange3dCR tileRange, DgnElementId entityId, TileDisplayParamsPtr& params, bool isCurved, DgnDbR db);
    //! Create a TileGeometry for an IBRepEntity
    static TileGeometryPtr Create(IBRepEntityR solid, TransformCR tf, DRange3dCR tileRange, DgnElementId entityId, TileDisplayParamsPtr& params, DgnDbR db);
    //! Create a TileGeometry for text.
    static TileGeometryPtr Create(TextStringR textString, TransformCR transform, DRange3dCR range, DgnElementId entityId, TileDisplayParamsPtr& params, DgnDbR db);
    //! Create a TileGeometry for a part instance.
    static TileGeometryPtr Create(TileGeomPartR part, TransformCR transform, DRange3dCR range, DgnElementId entityId, TileDisplayParamsPtr& params, DgnDbR db);

};
//=======================================================================================
// @bsistruct                                                   Ray.Bentley     12/2016
//=======================================================================================
struct TileGeomPart : RefCountedBase
{
private:
    DRange3d                m_range;
    TileGeometryList        m_geometries;
    size_t                  m_instanceCount;
    mutable size_t          m_facetCount;

    virtual uint32_t _GetExcessiveRefCountThreshold() const  override {return 100000;} 

protected:
    TileGeomPart(DRange3dCR range, TileGeometryList const& geometry);

public:
    static TileGeomPartPtr Create(DRange3dCR range, TileGeometryList const& geometry) { return new TileGeomPart(range, geometry); }
    TileGeometry::T_TilePolyfaces GetPolyfaces(IFacetOptionsR facetOptions, TileGeometryCR instance);
    TileGeometry::T_TileStrokes GetStrokes(IFacetOptionsR facetOptions, TileGeometryCR instance);
    size_t GetFacetCount(FacetCounter& counter) const;
    bool IsCurved() const;
    void IncrementInstanceCount() { m_instanceCount++; }
    size_t GetInstanceCount() const { return m_instanceCount; }

    TileGeometryList const& GetGeometries() const { return m_geometries; }
    DRange3d GetRange() const { return m_range; };
    bool IsWorthInstancing(double tolerance) const;
    

};  // TileGeomPart

//=======================================================================================
//! Filters elements during TileNode generation. Elements are selected according to their
//! intersection with a TileNode's range, then tested against the supplied ITileGenerationFilter
//! to apply additional selection criteria.
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct ITileGenerationFilter
{
protected:
    virtual bool _AcceptElement(DgnElementId elementId) const= 0;
public:
    //! Invoked for each element in the tile's range. Returns false to exclude the element from the tile geometry, or true to include it.
    bool AcceptElement(DgnElementId elementId) const { return _AcceptElement(elementId); }
};

//=======================================================================================            a
// @bsistruct                                                   Ray.Bentley     11/2016
//=======================================================================================
struct TileModelDelta : RefCountedBase, ITileGenerationFilter
    {
    struct ElementState
        {
        private:
        int64_t         m_lastModifiedTime;
        uint32_t        m_facetCount;

        public:
        ElementState() { }
        ElementState(int64_t lastModified,  uint32_t facetCount) : m_lastModifiedTime(lastModified), m_facetCount(facetCount) { }
        uint32_t GetFacetCount() const { return m_facetCount; }
        int64_t GetLastModifiedTime() const { return m_lastModifiedTime; }

        Json::Value GetJsonValue () const;
        void SetFacetCount(uint32_t facetCount) { m_facetCount = facetCount; }
        };


    void Save ();                         // Save to file.
    bool ChangesPresent() const { return m_deleted.empty() && m_added.empty(); }
    bool DoPublish(DgnElementId id) { return m_added.find(id) != m_added.end(); }
    ElementState* GetElementState(DgnElementId id) { auto const& found = m_elementStates.find(id); return found == m_elementStates.end() ? nullptr : &found->second; }
    static TileModelDeltaPtr Create(DgnModelCR model, BeFileNameCR dataDirectory) { return new TileModelDelta(model, dataDirectory); }
    bset<DgnElementId> const& GetAdded() const { return m_added; }
    bset<DgnElementId> const& GetDeleted() const { return m_deleted; }
    DGNPLATFORM_EXPORT bool DoIncremental (TileNodeCR tile) const;     // Return true if this previously generated tile is available and compatible with the current tile.


    virtual bool _AcceptElement(DgnElementId elementId) const override  { return m_added.find(elementId) != m_added.end(); }


private:
    BeFileName                          m_dataDirectory;

    BeFileName                          m_fileName;
    WString                             m_rootName;
    bmap<DgnElementId, ElementState>    m_elementStates;
    bset<DgnElementId>                  m_deleted;                  // A change will be handled as a delete/add.
    bset<DgnElementId>                  m_added;
    Json::Value                         m_previousTileSet;

protected:    
    TileModelDelta(DgnModelCR model, BeFileNameCR dataDirectory);       // Constructor for current model state.
    };

//=======================================================================================
//! Caches information used during tile generation.
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct TileGenerationCache : RefCountedBase
{
    // ###TODO: Put upper limit on sizes of geometry source/list caches...
    // The following options are mutually exclusive
    enum class Options
    {
        None = 0,               // cache nothing
        CacheGeometrySources,   // cache GeometrySources by element ID
        CacheGeometryLists,     // cache TileGeometryLists by element ID
    };
private:
    typedef bmap<DgnElementId, TileGeometryList>                    GeometryListMap;
    typedef std::map<DgnElementId, std::unique_ptr<GeometrySource>> GeometrySourceMap;

    DRange3d                    m_range;
    mutable GeometryListMap     m_geometry;
    mutable GeometrySourceMap   m_geometrySources;
    mutable BeMutex             m_mutex;    // for geometry cache
    mutable BeSQLite::BeDbMutex m_dbMutex;  // for multi-threaded access to database
    Options                     m_options;
    DgnModelPtr                 m_model;

    friend struct TileGenerator; // Invokes Populate() from ctor
    TileGenerationCache(Options options);
    TileGeneratorStatus Populate(DgnDbR db, DgnModelR model);

    static TileGenerationCachePtr Create(Options options = Options::None) { return new TileGenerationCache(options); }
public:
    DGNPLATFORM_EXPORT ~TileGenerationCache();

    DRange3dCR GetRange() const { return m_range; }
    DgnModelR GetModel() const { return *m_model; }

    bool WantCacheGeometrySources() const { return Options::CacheGeometrySources == m_options; }
    GeometrySourceCP GetCachedGeometrySource(DgnElementId elementId) const;
    GeometrySourceCP AddCachedGeometrySource(std::unique_ptr<GeometrySource>& source, DgnElementId elementId) const;

    bool WantCacheGeometry() const { return Options::CacheGeometryLists == m_options; }
    bool GetCachedGeometry(TileGeometryList& geometry, DgnElementId elementId) const;
    void AddCachedGeometry(DgnElementId elementId, TileGeometryList&& geometry) const;

     BeSQLite::BeDbMutex& GetDbMutex() const { return m_dbMutex; }
};

//=======================================================================================
// Represents the publishable geometry for a single tile
// @bsistruct                                                   Ray.Bentley     12/2016.
//=======================================================================================
struct PublishableTileGeometry
{
private:
    TileMeshList            m_meshes;
    TileMeshPartList        m_parts;
    TileMeshPointCloudList  m_pointClouds;


public:
    TileMeshList& Meshes()                  { return m_meshes; }
    TileMeshPartList& Parts()               { return m_parts; }
    TileMeshPointCloudList& PointClouds()   { return m_pointClouds; }
    bool IsEmpty() const                    { return m_meshes.empty() && m_parts.empty() && m_pointClouds.empty(); }

};  // PublishedTileGeometry

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
    virtual PublishableTileGeometry _GeneratePublishableGeometry(DgnDbR dgndb, TileGeometry::NormalMode normalMode=TileGeometry::NormalMode::CurvedSurfacesOnly, bool twoSidedTileTriangles=false, bool doSurfacesOnly=false, ITileGenerationFilterCP filter = nullptr) const = 0;
    virtual TileGeneratorStatus _CollectGeometry(TileGenerationCacheCR cache, DgnDbR db, TileModelDeltaP modelDelta, bool* leafThresholdExceeded, double tolerance, bool surfacesOnly, size_t leafCountThreshold) { return TileGeneratorStatus::Success; }
    virtual void _ClearGeometry() { }
    virtual TileModelDeltaCP _GetModelDelta() const { return nullptr; }
    virtual WString _GetFileExtension() const = 0;

public:
    DgnModelCR GetModel() const { return *m_model; }
    DRange3dCR GetDgnRange() const { return m_dgnRange; }
    DRange3d GetTileRange() const { DRange3d range = m_dgnRange; m_transformFromDgn.Multiply(range, range); return range; }
    DPoint3d GetTileCenter() const { DRange3d range = GetTileRange(); return DPoint3d::FromInterpolate(range.low, .5, range.high); }
    size_t GetDepth() const { return m_depth; } //!< This node's depth from the root tile node
    size_t GetSiblingIndex() const { return m_siblingIndex; } //!< This node's order within its siblings at the same depth
    double GetTolerance() const { return m_tolerance; }
    void SetTolerance(double tolerance) { m_tolerance = tolerance; }

    TileNodeCP GetParent() const { return m_parent; } //!< The direct parent of this node
    TileNodeP GetParent() { return m_parent; } //!< The direct parent of this node
    TileNodeCP GetRoot() const;
    TileNodeP GetRoot() { return const_cast<TileNodeP>(const_cast<TileNodeCP>(this)->GetRoot()); }
    TileNodeList const& GetChildren() const { return m_children; } //!< The direct children of this node
    TileNodeList& GetChildren() { return m_children; } //!< The direct children of this node
    void SetDgnRange(DRange3dCR range) { m_dgnRange = range; }
    void SetTileRange(DRange3dCR range) { Transform tf; DRange3d dgnRange = range; tf.InverseOf(m_transformFromDgn); tf.Multiply(dgnRange, dgnRange); SetDgnRange(dgnRange); }
    void SetPublishedRange(DRange3dCR publishedRange) const { m_publishedRange = publishedRange; }
    DRange3dCR GetPublishedRange() const { return m_publishedRange; }

    DGNPLATFORM_EXPORT size_t GetNodeCount() const;
    DGNPLATFORM_EXPORT size_t GetMaxDepth() const;
    DGNPLATFORM_EXPORT void GetTiles(TileNodePList& tiles);
    DGNPLATFORM_EXPORT TileNodePList GetTiles();
    DGNPLATFORM_EXPORT WString GetNameSuffix() const;
    DGNPLATFORM_EXPORT size_t   GetNameSuffixId() const;
    DGNPLATFORM_EXPORT WString GetFileName(WCharCP rootName, WCharCP extension) const;
    void SetIsEmpty(bool isEmpty) { m_isEmpty = isEmpty; }
    bool GetIsEmpty() const { return m_isEmpty; }

    TileGeneratorStatus  CollectGeometry(TileGenerationCacheCR cache, DgnDbR db, TileModelDeltaP modelDelta, bool* leafThresholdExceeded, double tolerance, bool surfacesOnly, size_t leafCountThreshold) { return _CollectGeometry(cache, db, modelDelta, leafThresholdExceeded, tolerance, surfacesOnly, leafCountThreshold); }
    void  ClearGeometry() { _ClearGeometry(); }
    TileSource GetSource() const { return _GetSource(); }
    TileModelDeltaCP GetModelDelta() const { return _GetModelDelta(); }
    WString GetFileExtension() const { return _GetFileExtension(); }
    PublishableTileGeometry GeneratePublishableGeometry(DgnDbR dgndb, TileGeometry::NormalMode normalMode=TileGeometry::NormalMode::CurvedSurfacesOnly, bool twoSidedTileTriangles=false, bool doSurfacesOnly=false, ITileGenerationFilterCP filter = nullptr) const
        { return _GeneratePublishableGeometry(dgndb, normalMode, twoSidedTileTriangles, doSurfacesOnly, filter); }
    DGNPLATFORM_EXPORT static void ComputeChildTileRanges(bvector<DRange3d>& subTileRanges, DRange3dCR range, size_t splitCount = 3);
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
    TileModelDeltaCP        m_modelDelta;
    mutable bool            m_containsParts;

    virtual uint32_t _GetExcessiveRefCountThreshold() const  override {return 100000;} // A deep tree can trigger this assertion erroneously.


    TileMeshList GenerateMeshes(DgnDbR db, TileGeometry::NormalMode normalMode, bool twoSidedTriangles, bool doSurfacesOnly, bool doRangeTest, ITileGenerationFilterCP filter, TileGeometryList const& geometries) const;

protected:
    ElementTileNode(DgnModelCR model, TileModelDeltaCP modelDelta, TransformCR transformFromDgn) : TileNode(model, transformFromDgn), m_isLeaf(false), m_modelDelta(modelDelta), m_containsParts(false) { }
    ElementTileNode(DgnModelCR model, TileModelDeltaCP modelDelta, DRange3dCR range, TransformCR transformFromDgn, size_t depth, size_t siblingIndex, TileNodeP parent, double tolerance = 0.0)
        : TileNode(model, range, transformFromDgn, depth, siblingIndex, parent, tolerance), m_isLeaf(false), m_modelDelta(modelDelta), m_containsParts(false) { }


    DGNPLATFORM_EXPORT virtual PublishableTileGeometry _GeneratePublishableGeometry(DgnDbR, TileGeometry::NormalMode, bool, bool, ITileGenerationFilterCP filter) const override;
    virtual TileSource _GetSource() const override final { return TileSource::Element; }
    virtual TileGeneratorStatus _CollectGeometry(TileGenerationCacheCR cache, DgnDbR db, TileModelDeltaP modelDelta, bool* leafThresholdExceeded, double tolerance, bool surfacesOnly, size_t leafCountThreshold) override;
    virtual void _ClearGeometry() override { m_geometries.clear(); }
    virtual TileModelDeltaCP _GetModelDelta() const { return m_modelDelta; }
    virtual WString _GetFileExtension() const override { return m_containsParts ? L"cmpt" : L"b3dm"; }

public:
    static ElementTileNodePtr Create(DgnModelCR model, TileModelDeltaCP modelDelta, TransformCR transformFromDgn) { return new ElementTileNode(model, modelDelta, transformFromDgn); }
    static ElementTileNodePtr Create(DgnModelCR model, TileModelDeltaCP modelDelta, DRange3dCR dgnRange, TransformCR transformFromDgn, size_t depth, size_t siblingIndex, TileNodeP parent)
        { return new ElementTileNode(model, modelDelta, dgnRange, transformFromDgn, depth, siblingIndex, parent); }

    void AdjustTolerance(double newTolerance);
    void SetIsLeaf(bool isLeaf) { m_isLeaf = isLeaf; }
    TileGeometryList const& GetGeometries() const { return m_geometries; }

};

//=======================================================================================
//! A TileNode generated from a model implementing IGenerateMeshTiles.
//! This does not include geometry of elements within the model.
// @bsistruct                                                   Paul.Connelly   10/16
//=======================================================================================
struct ModelTileNode : TileNode
{
    bool        m_pointCloud;

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
    //! Interface adopted by an object which collects generated tiles
    struct EXPORT_VTABLE_ATTRIBUTE ITileCollector
        {
        //! Invoked from one of several worker threads for each generated tile.
        virtual TileGeneratorStatus _AcceptTile(TileNodeCR tileNode) = 0;
        //! Invoked before a model is processed.
        virtual TileGeneratorStatus _BeginProcessModel(DgnModelCR model) { return TileGeneratorStatus::Success; }
        //! Invoked after a model is processed, with the result of processing.
        virtual TileGeneratorStatus _EndProcessModel(DgnModelCR model, TileNodeP rootTile, TileGeneratorStatus status) { return status; }
        //! Return true to invoke incremental model publishing.
        virtual bool _DoIncrementalModelPublish (BeFileNameR dataDirectory, DgnModelCR model) { return false; }
        };

    //! Accumulates statistics during tile generation
    struct Statistics
        {
        size_t      m_tileCount = 0;
        double      m_tileGenerationTime = 0.0;
        };
            
        
private:
    Statistics                              m_statistics;
    ITileGenerationProgressMonitorR         m_progressMeter;
    Transform                               m_transformFromDgn;
    DgnDbR                                  m_dgndb;
    BeAtomic<uint32_t>                      m_totalTiles;
    uint32_t                                m_totalModels;
    BeAtomic<uint32_t>                      m_completedModels;
    bmap<DgnModelId, TileModelDeltaPtr>     m_modelDeltas;

    struct ElementTileContext
        {
        DgnPlatformLib::Host&   m_host;
        TileGenerationCachePtr  m_cache;
        DgnModelPtr             m_model;
        ITileCollector*         m_collector;
        double                  m_leafTolerance;
        bool                    m_surfacesOnly;
        size_t                  m_maxPointsPerTile;
        TileModelDeltaP         m_modelDelta;


        ElementTileContext(TileGenerationCacheR cache, DgnModelR model, TileModelDeltaP modelDelta, ITileCollector& collector, double leafTolerance, bool surfacesOnly, size_t maxPointsPerTile)
            : m_host(T_HOST), m_cache(&cache), m_model(&model), m_collector(&collector), m_leafTolerance(leafTolerance), m_maxPointsPerTile(maxPointsPerTile), m_modelDelta(modelDelta), m_surfacesOnly(surfacesOnly) { }
        };

    struct ElementTileResult
        {
        ElementTileNodePtr      m_tile;
        TileGeneratorStatus     m_status;

        explicit ElementTileResult(TileGeneratorStatus status, ElementTileNodeP tile=nullptr) : m_tile(tile), m_status(status)
            { BeAssert(TileGeneratorStatus::Success != m_status || m_tile.IsValid()); }
        };

    typedef folly::Future<TileGeneratorStatus> FutureStatus;
    typedef folly::Future<ElementTileResult> FutureElementTileResult;

    FutureElementTileResult GenerateElementTiles(ITileCollector& collector, double leafTolerance, bool surfacesOnly, size_t maxPointsPerTile, DgnModelR model, TileModelDeltaP modelDelta);
    FutureStatus PopulateCache(ElementTileContext context);
    FutureElementTileResult GenerateTileset(TileGeneratorStatus status, ElementTileContext context);
    FutureElementTileResult ProcessParentTile(ElementTileNodePtr parent, ElementTileContext context);
    FutureElementTileResult ProcessChildTiles(TileGeneratorStatus status, ElementTileNodePtr parent, ElementTileContext context);
                                                                                
    FutureStatus GenerateTiles(ITileCollector& collector, double leafTolerance, bool surfacesOnly, size_t maxPointsPerTile, DgnModelR model);
    FutureStatus GenerateTilesFromModels(ITileCollector& collector, DgnModelIdSet const& modelIds, double leafTolerance, bool surfacesOnly, size_t maxPointsPerTile);

public:
    DGNPLATFORM_EXPORT explicit TileGenerator(TransformCR transformFromDgn, DgnDbR dgndb, ITileGenerationFilterP filter=nullptr, ITileGenerationProgressMonitorP progress=nullptr);

    DgnDbR GetDgnDb() const { return m_dgndb; }
    TransformCR GetTransformFromDgn() const { return m_transformFromDgn; }
    Statistics const& GetStatistics() const { return m_statistics; }
    ITileGenerationProgressMonitorR GetProgressMeter() { return m_progressMeter; }
    TileModelDeltaPtr GetTileModelDelta(DgnModelId modelId) { auto const& found = m_modelDeltas.find(modelId); return found == m_modelDeltas.end() ? nullptr : found->second; }

    DGNPLATFORM_EXPORT TileGeneratorStatus GenerateTiles(ITileCollector& collector, DgnModelIdSet const& modelIds, double leafTolerance, bool surfacesOnly, size_t maxPointsPerTile);
};

//=======================================================================================
// Interface for models to generate HLOD tree of TileNodes 
// @bsistruct                                                   Ray.Bentley     08/2016
//=======================================================================================
struct IGenerateMeshTiles
{
    virtual TileGeneratorStatus _GenerateMeshTiles(TileNodePtr& rootTile, TransformCR transformDbToTile, double leafTolerance, TileGenerator::ITileCollector& collector, ITileGenerationProgressMonitorR progressMeter) = 0;

};  // IPublishModelMeshTiles


//=======================================================================================
// static utility methods
// @bsistruct                                                   Ray.Bentley     08/2016
//=======================================================================================
struct TileUtil
{
    DGNPLATFORM_EXPORT static BentleyStatus WriteJsonToFile (WCharCP fileName, Json::Value const& value);
    DGNPLATFORM_EXPORT static BentleyStatus ReadJsonFromFile (Json::Value& value, WCharCP fileName);
    DGNPLATFORM_EXPORT static WString GetRootNameForModel(DgnModelCR model);

};  // TileUtil

END_BENTLEY_RENDER_NAMESPACE


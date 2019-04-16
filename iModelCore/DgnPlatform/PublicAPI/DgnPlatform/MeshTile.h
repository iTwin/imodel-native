/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#define CLASSIFICATION_WIP
// #define WIP_MESHTILE_3SM

#pragma once
/*__PUBLISH_SECTION_START__*/

#include "Render.h"
#include "DgnTexture.h"
#include "DgnMaterial.h"
#include "SolidKernel.h"
#include "RangeIndex.h"
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
BENTLEY_RENDER_TYPEDEFS(TileDisplayParamsCache);
BENTLEY_RENDER_TYPEDEFS(TileTextureImage);
BENTLEY_RENDER_TYPEDEFS(ITileGenerationFilter);
BENTLEY_RENDER_TYPEDEFS(ITileCollectionFilter);
BENTLEY_RENDER_TYPEDEFS(TileGenerationCache);
BENTLEY_RENDER_TYPEDEFS(ITileGenerationProgressMonitor);
BENTLEY_RENDER_TYPEDEFS(TileGeomPart);
BENTLEY_RENDER_TYPEDEFS(PublishableTileGeometry);
BENTLEY_RENDER_TYPEDEFS(FeatureAttributes);
BENTLEY_RENDER_TYPEDEFS(FeatureAttributesMap);
BENTLEY_RENDER_TYPEDEFS(ColorIndexMap);
BENTLEY_RENDER_TYPEDEFS(IGetTileTreeForPublishing);
BENTLEY_RENDER_TYPEDEFS(TileTreePublishRenderSystem);
BENTLEY_RENDER_TYPEDEFS(IGetPublishedTilesetInfo);
BENTLEY_RENDER_TYPEDEFS(ICustomTilesetPublisher);

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
// ! Holds a texture image.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct TileTextureImage : RefCountedBase
    {
    private:
        ImageSource         m_imageSource;
        bool                m_repeat;

        TileTextureImage(ImageSource&& imageSource, bool repeat) : m_imageSource(std::move(imageSource)), m_repeat(repeat) { BeAssert(m_imageSource.IsValid()); }
        TileTextureImage(ImageSource& imageSource, bool repeat) : m_imageSource (imageSource), m_repeat(repeat) { BeAssert(m_imageSource.IsValid()); }
    public:
        static TileTextureImagePtr Create(ImageSource&& imageSource, bool repeat=true) { return new TileTextureImage(std::move(imageSource), repeat); }
        static TileTextureImagePtr Create(ImageSource& imageSource, bool repeat=true) { return new TileTextureImage(imageSource, repeat); }
        static TileTextureImagePtr Create(GradientSymbCR gradient);
        static ImageSource Load(TileDisplayParamsCR params, DgnDbR db);

        ImageSourceCR GetImageSource() const { return m_imageSource; }
        bool GetRepeat() const { return m_repeat; }
    };

//=======================================================================================
//! Display params associated with TileGeometry. Based on GraphicParams and GeometryParams.
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct TileDisplayParams : RefCountedBase
{
    friend struct TileDisplayParamsCache;

private:
    DgnCategoryId                   m_categoryId;
    DgnSubCategoryId                m_subCategoryId;
    uint32_t                        m_color;
    mutable TileTextureImagePtr     m_textureImage;
    DgnGeometryClass                m_class = DgnGeometryClass::Primary;
    bool                            m_isColorFromBackground = false;
    // Mesh only...
    RenderMaterialId                m_materialId;
    bool                            m_ignoreLighting;
    GradientSymbPtr                 m_gradient;
    // Polyline only...
    uint32_t                        m_rasterWidth;  
    uint32_t                        m_linePixels;   

    uint32_t _GetExcessiveRefCountThreshold() const override {return 0x7fffffff;} 

    DGNPLATFORM_EXPORT TileDisplayParams(GraphicParamsCP graphicParams, GeometryParamsCP geometryParams, bool ignoreLighting, bool useLineParams);
    TileDisplayParams(uint32_t color, TileTextureImageP texture, bool ignoreLighting) : m_color(color), m_textureImage(texture), m_ignoreLighting(ignoreLighting), m_rasterWidth(0), m_linePixels(0) { }
    TileDisplayParams(uint32_t color, GeometryParamsCR geometryParams) : m_color(color), m_ignoreLighting(false), m_materialId(geometryParams.GetMaterialId()), m_rasterWidth(0), m_linePixels(0), m_class(geometryParams.GetGeometryClass()) {}
    TileDisplayParams(uint32_t color, RenderMaterialId materialId) : m_color(color), m_materialId(materialId), m_ignoreLighting(false), m_rasterWidth(0), m_linePixels(0) {}
    TileDisplayParams() : TileDisplayParams(nullptr, nullptr, false, false) {}

    TileDisplayParamsCPtr Clone() const;
public:
    static TileDisplayParamsCPtr Create(uint32_t color, GeometryParamsCR geomParams) { return new TileDisplayParams(color, geomParams); }
    static TileDisplayParamsCPtr Create(uint32_t color, TileTextureImageP texture, bool ignoreLighting) { return new TileDisplayParams(color, texture, ignoreLighting); }
    static TileDisplayParamsCPtr Create() { return new TileDisplayParams(); }
    static TileDisplayParamsCPtr CreateForAttachment(GraphicParamsCR gfParams, GeometryParamsCR geomParams, TileTextureImageR texture);

    // These comparisons ignore category, subcategory, and class.
    bool operator<(TileDisplayParamsCR rhs) const { return IsLessThan(rhs, true); }
    bool operator==(TileDisplayParamsCR rhs) const { return !(*this != rhs); }
    bool operator!=(TileDisplayParamsCR rhs) const { return *this < rhs || rhs < *this; }
    DGNPLATFORM_EXPORT bool IsLessThan(TileDisplayParams const& rhs, bool compareColors) const;

    DgnCategoryId GetCategoryId() const { return m_categoryId; }
    DgnSubCategoryId GetSubCategoryId() const { return m_subCategoryId; }
    RenderMaterialId GetRenderMaterialId() const { return m_materialId; }
    DgnGeometryClass GetClass() const { return m_class; }
    uint32_t GetColor() const { return m_color; }
    uint32_t GetRasterWidth() const { return m_rasterWidth; }
    uint32_t GetLinePixels() const { return m_linePixels; }
    bool GetIgnoreLighting() const { return m_ignoreLighting; }
    bool GetIsColorFromBackground() const { return m_isColorFromBackground; }
    DgnTextureCPtr QueryTexture(DgnDbR db) const;
    TileTextureImagePtr& TextureImage() { return m_textureImage; }
    TileTextureImageCP GetTextureImage() const { return m_textureImage.get(); }
    GradientSymbCP GetGradient() const { return m_gradient.get(); }
    DGNPLATFORM_EXPORT void ResolveTextureImage(DgnDbR db) const;
    bool HasTexture(DgnDbR db) const { return m_gradient.IsValid() || m_textureImage.IsValid() || QueryTexture(db).IsValid(); }

    // These comparisons ignore category, subcategory, and class.
    DGNPLATFORM_EXPORT bool IsStrictlyLessThan(TileDisplayParamsCR rhs) const;
    DGNPLATFORM_EXPORT bool IsStrictlyEqualTo(TileDisplayParamsCR rhs) const;
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   03/17
//=======================================================================================
struct TileDisplayParamsCache
{
private:
    struct Comparator
    {
        bool operator()(TileDisplayParamsCPtr const& lhs, TileDisplayParamsCPtr const& rhs) const
            {
            return lhs->IsStrictlyLessThan(*rhs);
            }
    };

    typedef bset<TileDisplayParamsCPtr, Comparator> Set;

    Set     m_set;

    TileDisplayParamsCR Get(TileDisplayParamsR params);
public:
    TileDisplayParamsCR Get(GraphicParamsCR gfParams, GeometryParamsCR geomParams, bool ignoreLighting=false, bool useLineParams=false)
        {
        TileDisplayParams params(&gfParams, geomParams.IsResolved() ? &geomParams : nullptr, ignoreLighting, useLineParams);
        return Get(params);
        }
    TileDisplayParamsCR GetDefault()
        {
        TileDisplayParams params;
        return Get(params);
        }
    TileDisplayParamsCR Get(uint32_t fill, TileTextureImageP textureImage, bool ignoreLighting)
        {
        TileDisplayParams params(fill, textureImage, ignoreLighting);
        return Get(params);
        }
    TileDisplayParamsCR Get(uint32_t fill, GeometryParamsCR geomParams)
        {
        TileDisplayParams params(fill, geomParams);
        return Get(params);
        }
};

//=======================================================================================
//! For a tile, records unique attribute values and associates each with a 16-bit index.
// @bsistruct                                                   Paul.Connelly   02/17
//=======================================================================================
template<typename T, typename INDEX> struct AttributeIndexMap
{
    typedef bmap<T, INDEX> Map;
protected:
    Map     m_map;

    static constexpr INDEX GetMaxIndex() { return static_cast <INDEX> (-1); }

    INDEX GetOrCreateIndex(T const& value)
        {
        auto iter = m_map.find(value);
        if (m_map.end() != iter)
            return iter->second;
        else if (IsFull())
            { BeAssert(false); return 0; }

        auto index = GetNumIndices();
        m_map[value] = index;
        return index;
        }
public:
    bool IsFull() const { return m_map.size() >= GetMaxIndex(); }
    INDEX GetNumIndices() const { return static_cast<INDEX>(size()); }

    typedef typename Map::const_iterator const_iterator;
    typedef const_iterator iterator;

    const_iterator begin() const { return m_map.begin(); }
    const_iterator end() const { return m_map.end(); }
    size_t size() const { return m_map.size(); }
    bool empty() const { return m_map.empty(); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   02/17
//=======================================================================================
struct ColorIndexMap : AttributeIndexMap<uint32_t, uint16_t>
{
private:
    bool    m_hasAlpha = false;
public:
    uint16_t GetIndex(uint32_t color)
        {
        BeAssert(!IsFull());
        BeAssert(empty() || (0 != ColorDef(color).GetAlpha()) == m_hasAlpha);
        m_hasAlpha |= (!IsFull() && 0 != ColorDef(color).GetAlpha());
        return GetOrCreateIndex(color);
        }

    bool HasTransparency() const { return m_hasAlpha; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   02/17
//=======================================================================================
struct FeatureAttributes
{
private:
    DgnElementId        m_elementId;
    DgnSubCategoryId    m_subCategoryId;
    DgnGeometryClass    m_class;

    bool AreIdsValid() const { /* BeAssert(m_elementId.IsValid() == m_subCategoryId.IsValid()); */ return m_elementId.IsValid();  }
public:
    FeatureAttributes() : m_class(DgnGeometryClass::Primary) { }
    FeatureAttributes(DgnElementId elementId, DgnSubCategoryId subCatId, DgnGeometryClass geomClass) : m_elementId(elementId), m_subCategoryId(subCatId), m_class(geomClass) { }
    FeatureAttributes(DgnElementId elementId, TileDisplayParamsCR params) : FeatureAttributes(elementId, params.GetSubCategoryId(), params.GetClass()) { }

    DgnElementId GetElementId() const { return m_elementId; }
    DgnSubCategoryId GetSubCategoryId() const { return m_subCategoryId; }
    DgnGeometryClass GetClass() const { return m_class; }

    bool operator==(FeatureAttributesCR rhs) const
        {
        if (IsUndefined() && rhs.IsUndefined())
            return true;
        else
            return GetElementId() == rhs.GetElementId() && GetSubCategoryId() == rhs.GetSubCategoryId() && GetClass() == rhs.GetClass();
        }

    DGNPLATFORM_EXPORT bool operator<(FeatureAttributesCR rhs) const;

    bool IsDefined() const { BeAssert(AreIdsValid() || DgnGeometryClass::Primary == GetClass()); return AreIdsValid(); }
    bool IsUndefined() const { return !IsDefined(); }
};

//=======================================================================================
//! For a tile, records unique vertex attributes and associates each with an index.
//! The index is 16 bits because we limit geometry IDs to 0xffff per-tile
//! Index 0 always corresponds to an undefined attribute.
// @bsistruct                                                   Paul.Connelly   02/17
//=======================================================================================
struct FeatureAttributesMap : AttributeIndexMap<FeatureAttributes, uint32_t>
{
//    DEFINE_T_SUPER(AttributeIndexMap<FeatureAttributes, uint32_t>);
public:
    DGNPLATFORM_EXPORT FeatureAttributesMap();

    uint32_t GetIndex(FeatureAttributesCR value) { return GetOrCreateIndex(value); }
    DGNPLATFORM_EXPORT uint32_t GetIndex(TileGeometryCR geom);
    uint32_t GetIndex(DgnElementId id, TileDisplayParamsCR params) { return GetIndex(FeatureAttributes(id, params)); }

    bool AnyDefined() const { BeAssert(m_map.size() > 0 && m_map.size() <= GetMaxIndex()); return m_map.size() > 1; }
    DGNPLATFORM_EXPORT void RemoveUndefined();
};

//=======================================================================================
//! Represents a mesh instance.
// @bsistruct                                                   Ray.Bentley     12/2016
//=======================================================================================
struct TileMeshInstance
{
private:
    FeatureAttributes   m_attributes;
    Transform           m_transform;

public:
    FeatureAttributesCR GetAttributes() const { return m_attributes; }
    TransformCR GetTransform() const { return m_transform; }
    TileMeshInstance (FeatureAttributesCR attributes, TransformCR transform) : m_attributes(attributes), m_transform(transform) { }
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

    uint32_t _GetExcessiveRefCountThreshold() const override {return 100000;} \

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
    bool        m_edgeVisible[3];

    explicit TileTriangle(bool singleSided=true) : m_singleSided(singleSided) { SetIndices(0, 0, 0); SetVisibility(true);  }
    TileTriangle(uint32_t indices[3], bool singleSided) : m_singleSided(singleSided) { SetIndices(indices);  SetVisibility(true); }
    TileTriangle(uint32_t a, uint32_t b, uint32_t c, bool singleSided) : m_singleSided(singleSided) { SetIndices(a, b, c);  SetVisibility(true); }

    void SetIndices(uint32_t indices[3]) { SetIndices(indices[0], indices[1], indices[2]); }
    void SetIndices(uint32_t a, uint32_t b, uint32_t c) { m_indices[0] = a; m_indices[1] = b; m_indices[2] = c; }
    void SetVisibility(bool visible) { m_edgeVisible[0] = m_edgeVisible[1] = m_edgeVisible[2] = visible; }
    bool IsSingleSided() const { return m_singleSided; }
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
    TileDisplayParamsCPtr       m_displayParams;
    bvector<TileTriangle>       m_triangles;
    bvector<TilePolyline>       m_polylines;
    bvector<DPoint3d>           m_points;
    bvector<DVec3d>             m_normals;
    bvector<DPoint2d>           m_uvParams;
    bvector<uint32_t>           m_attributes;
    bvector<uint16_t>           m_colors;
    ColorIndexMap               m_colorIndex;
    bool                        m_validIdsPresent = false;

    TileMesh(TileDisplayParamsCR params) : m_displayParams(&params) { }
    template<typename T> T const* GetMember(bvector<T> const& from, uint32_t at) const { return at < from.size() ? &from[at] : nullptr; }
public:
    static TileMeshPtr Create(TileDisplayParamsCR params) { return new TileMesh(params); }

    DGNPLATFORM_EXPORT DRange3d GetTriangleRange(TileTriangleCR triangle) const;
    DGNPLATFORM_EXPORT DVec3d GetTriangleNormal(TileTriangleCR triangle) const;
    DGNPLATFORM_EXPORT bool HasNonPlanarNormals() const;
    DGNPLATFORM_EXPORT bool RemoveEntityGeometry(bset<DgnElementId> const& ids);

    TileDisplayParamsCR GetDisplayParams() const { return *m_displayParams; } //!< The mesh symbology
    TileDisplayParamsCPtr GetDisplayParamsPtr() const { return m_displayParams; } //!< The mesh symbology
    bvector<TileTriangle> const& Triangles() const { return m_triangles; } //!< TileTriangles defined as a set of 3 indices into the vertex attribute arrays.
    bvector<TilePolyline> const& Polylines() const { return m_polylines; } //!< Polylines defined as a set of indices into the vertex attribute arrays.
    bvector<DPoint3d> const& Points() const { return m_points; } //!< Position vertex attribute array
    bvector<DVec3d> const& Normals() const { return m_normals; } //!< Normal vertex attribute array
    bvector<DPoint2d> const& Params() const { return m_uvParams; } //!< UV params vertex attribute array
    bvector<uint32_t> const& Attributes() const { return m_attributes; } //!< Vertex attribute array specifying the subcategory, element, and class associated with the vertex
    bvector<uint16_t> const& Colors() const { return m_colors; } //!< Indices into ColorIndexMap, for untextured meshes
    bvector<DPoint3d>& PointsR() { return m_points; } //!< Position vertex attribute array
    bvector<DVec3d>& NormalsR() { return m_normals; } //!< Normal vertex attribute array
    bvector<DPoint2d>& ParamsR() { return m_uvParams; } //!< UV params vertex attribute array
    bvector<uint32_t>& AttributesR() { return m_attributes; }
    bvector<uint16_t>& ColorsR() { return m_colors; }
    ColorIndexMap const& GetColorIndexMap() const { return m_colorIndex; }

    TileTriangleCP GetTriangle(uint32_t index) const { return GetMember(m_triangles, index); }
    DPoint3dCP GetPoint(uint32_t index) const { return GetMember(m_points, index); }
    DVec3dCP GetNormal(uint32_t index) const { return GetMember(m_normals, index); }
    DPoint2dCP GetParam(uint32_t index) const { return GetMember(m_uvParams, index); }

    uint32_t GetAttributesIndex(uint32_t index) const { auto pAttr = GetMember(m_attributes, index); return nullptr != pAttr ? *pAttr : 0; }
    uint32_t GetTriangleAttributesIndex(TileTriangleCR triangle) const { return GetAttributesIndex(triangle.m_indices[0]); }
    uint32_t GetPolylineAttributesIndex(TilePolylineCR polyline) const { return polyline.GetIndices().empty() ? 0 : GetAttributesIndex(polyline.GetIndices().front()); }

    bool IsEmpty() const { return m_triangles.empty() && m_polylines.empty(); }
    DRange3d GetRange() const { return DRange3d::From(m_points); }
    DRange3d GetUVRange() const { return DRange3d::From(m_uvParams, 0.0); }

    bool ValidIdsPresent() const { return m_validIdsPresent; }

    void AddTriangle(TileTriangleCR triangle) { m_triangles.push_back(triangle); }
    void AddPolyline(TilePolyline polyline) { m_polylines.push_back(polyline); }
    void AddRenderTile(Render::GraphicBuilder::TileCorners const&, TransformCR transform);
    void AddTriMesh(Render::TriMeshArgsCR triMesh, TransformCR transform, bool invertVParam);
    
    DGNPLATFORM_EXPORT void AddMesh(TileMeshCR mesh);
    DGNPLATFORM_EXPORT uint32_t AddVertex(DPoint3dCR point, DVec3dCP normal, DPoint2dCP param, uint32_t attribute, uint32_t color);
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
    bool                m_hasBatchIds;

    TileMeshMergeKey() : m_params(nullptr), m_hasNormals(false), m_hasFacets(false), m_hasBatchIds(false) { }
    TileMeshMergeKey(TileDisplayParamsCR params, bool hasNormals, bool hasFacets, bool hasBatchIds) : m_params(&params), m_hasNormals(hasNormals), m_hasFacets(hasFacets), m_hasBatchIds(hasBatchIds) { }
    TileMeshMergeKey(TileMeshCR mesh) : TileMeshMergeKey(mesh.GetDisplayParams(),  !mesh.Normals().empty(), !mesh.Triangles().empty(), mesh.ValidIdsPresent()) { }

    bool operator<(TileMeshMergeKey const& rhs) const
        {
        BeAssert(nullptr != m_params && nullptr != rhs.m_params);
        if(m_hasNormals != rhs.m_hasNormals)
            return !m_hasNormals;

        if(m_hasFacets != rhs.m_hasFacets)
            return !m_hasFacets;

        if (m_hasBatchIds != rhs.m_hasBatchIds)
            return !m_hasBatchIds;

        return m_params->IsLessThan(*rhs.m_params, false);  // don't compare colors.
        }
};


//=======================================================================================
//! Builds a single TileMesh to a specified level of detail.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct TileMeshBuilder : RefCountedBase
{
    struct VertexKey
        {
        DPoint3d            m_point;
        DVec3d              m_normal;
        DPoint2d            m_param;
        FeatureAttributes   m_attributes;
        uint32_t            m_color = 0;
        bool                m_normalValid = false;
        bool                m_paramValid = false;

        VertexKey() { }
        VertexKey(DPoint3dCR point, DVec3dCP normal, DPoint2dCP param, FeatureAttributesCR attr, uint32_t color) : m_point(point), m_normalValid(nullptr != normal), m_paramValid(nullptr != param), m_attributes(attr), m_color(color)
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
    VertexMap               m_vertexMap;
    TileTriangleSet         m_triangleSet;
    double                  m_tolerance;
    double                  m_areaTolerance;
    size_t                  m_triangleIndex;
    RenderMaterialCPtr      m_materialEl;
    RenderingAssetCP        m_material = nullptr;
    FeatureAttributesMapR   m_attributes;
    Transform               m_transformToDgn;

    bool GetMaterial(RenderMaterialId materialId, DgnDbR dgnDb);
    TileMeshBuilder(TileDisplayParamsCR params, TransformCR transformFromDgn, double tolerance, double areaTolerance, FeatureAttributesMapR attr) : m_mesh(TileMesh::Create(params)), m_vertexMap(VertexKey::Comparator(tolerance * 1.0E-3)), 
            m_tolerance(tolerance), m_areaTolerance(areaTolerance), m_triangleIndex(0), m_attributes(attr) { m_transformToDgn.InverseOf(transformFromDgn); }
public:
    static TileMeshBuilderPtr Create(TileDisplayParamsCR params, TransformCR transformFromDgn, double tolerance, double areaTolerance, FeatureAttributesMapR attr) { return new TileMeshBuilder(params, transformFromDgn, tolerance, areaTolerance, attr); }

    DGNPLATFORM_EXPORT void AddTriangle(PolyfaceVisitorR visitor, RenderMaterialId materialId, DgnDbR dgnDb, FeatureAttributesCR attr, bool includeParams, uint32_t color, bool requireNormals);
    DGNPLATFORM_EXPORT void AddPolyline(bvector<DPoint3d>const& polyline, FeatureAttributesCR attr, uint32_t color);
    DGNPLATFORM_EXPORT void AddPolyface(PolyfaceQueryCR polyface, RenderMaterialId materialId, DgnDbR dgnDb, FeatureAttributesCR attr, bool includeParams, uint32_t color);

    void AddMesh(TileTriangleCR triangle);
    void AddTriangle(TileTriangleCR triangle);
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

struct Rgb
    {
    uint8_t m_red;
    uint8_t m_green;
    uint8_t m_blue;
    };


private:
    TileDisplayParamsCPtr   m_displayParams;
    bvector<DPoint3d>       m_points;
    bvector<Rgb>            m_colors;       // Color565
    bvector<FPoint3d>       m_normals;
    
    TileMeshPointCloud (TileDisplayParamsCR params, DPoint3dCP points, Rgb const* colors, FPoint3dCP normals, size_t nPoints, TransformCR transform, double clusterTolerance);

public:
    bvector<DPoint3d> const& Points()       { return m_points; }
    bvector<Rgb> const& Colors()            { return m_colors; }
    bvector<FPoint3d> const& Normals()      { return m_normals; }
    DGNPLATFORM_EXPORT static TileMeshPointCloudPtr Create(TileDisplayParamsCR params, DPoint3dCP points, Rgb const* colors, FPoint3dCP normals, size_t nPoints, TransformCR transform, double clusterTolerance) { return new TileMeshPointCloud (params, points,  colors, normals, nPoints, transform, clusterTolerance); }

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
        TileDisplayParamsCPtr   m_displayParams;
        PolyfaceHeaderPtr       m_polyface;
            
        TilePolyface(TileDisplayParamsCR displayParams, PolyfaceHeaderR polyface) : m_displayParams(&displayParams), m_polyface(&polyface) { }
        void Transform(TransformCR transform) { if (m_polyface.IsValid()) m_polyface->Transform (transform); }
        TilePolyface    Clone() const { return TilePolyface(*m_displayParams, *m_polyface->Clone()); }
        };
    struct TileStrokes
        {
        TileDisplayParamsCPtr       m_displayParams;
        bvector<bvector<DPoint3d>>  m_strokes;
        bool                        m_disjoint;

        void Transform(TransformCR transform);

        TileStrokes (TileDisplayParamsCR displayParams, bvector<bvector<DPoint3d>>&& strokes, bool disjoint = false) : m_displayParams(&displayParams),  m_strokes(std::move(strokes)), m_disjoint(disjoint) { }
        TileStrokes (TileDisplayParamsCR displayParams, bool disjoint = false) : m_displayParams(&displayParams), m_disjoint(disjoint) { }
        }; 

    typedef bvector<TilePolyface>   T_TilePolyfaces;
    typedef bvector<TileStrokes>    T_TileStrokes;

private:
    TileDisplayParamsCPtr   m_params;
    Transform               m_transform;
    DRange3d                m_tileRange;
    DgnElementId            m_entityId;
    bool                    m_isCurved;
    bool                    m_hasTexture;
    mutable size_t          m_facetCount;


protected:
    TileGeometry(TransformCR tf, DRange3dCR tileRange, DgnElementId entityId, TileDisplayParamsCR params, bool isCurved, DgnDbR db);

    virtual T_TilePolyfaces _GetPolyfaces(IFacetOptionsR facetOptions) = 0;
    virtual T_TileStrokes _GetStrokes (IFacetOptionsR facetOptions) { return T_TileStrokes(); }
    virtual bool _DoDecimate() const { return false; }
    virtual size_t _GetFacetCount(FacetCounter& counter) const = 0;
    virtual TileGeomPartCPtr _GetPart() const { return TileGeomPartCPtr(); }
    virtual bool _IsPoint() const { return false; }     

    void SetFacetCount(size_t numFacets);
public:
    TileDisplayParamsCR GetDisplayParams() const { return *m_params; }
    TransformCR GetTransform() const { return m_transform; }
    DRange3dCR GetTileRange() const { return m_tileRange; }
    DgnElementId GetEntityId() const { return m_entityId; } //!< The ID of the element from which this geometry was produced
    size_t GetFacetCount(IFacetOptionsR options) const;
    size_t GetFacetCount(FacetCounter& counter) const { return _GetFacetCount(counter); }
    virtual bool _IsViewDependent() const { return false; }

    FeatureAttributes GetAttributes() const { return m_params.IsValid() ? FeatureAttributes(GetEntityId(), *m_params) : FeatureAttributes(); }
    
    IFacetOptionsPtr CreateFacetOptions(double chordTolerance, NormalMode normalMode) const;

    bool IsCurved() const { return m_isCurved; }
    bool HasTexture() const { return m_hasTexture; }
    bool IsPoint() const { return _IsPoint(); }

    T_TilePolyfaces GetPolyfaces(IFacetOptionsR facetOptions) { return _GetPolyfaces(facetOptions); }
    T_TilePolyfaces GetPolyfaces(double chordTolerance, NormalMode normalMode);
    bool DoDecimate() const { return _DoDecimate(); }
    T_TileStrokes GetStrokes (IFacetOptionsR facetOptions) { return _GetStrokes(facetOptions); }
    TileGeomPartCPtr GetPart() const { return _GetPart(); }


    //! Create a TileGeometry for an IGeometry
    static TileGeometryPtr Create(IGeometryR geometry, TransformCR tf, DRange3dCR tileRange, DgnElementId entityId, TileDisplayParamsCR params, bool isCurved, bool curvesAsWire, DgnDbR db);
    //! Create a TileGeometry for an IBRepEntity
    static TileGeometryPtr Create(IBRepEntityR solid, TransformCR tf, DRange3dCR tileRange, DgnElementId entityId, TileDisplayParamsCR params, DgnDbR db);
    //! Create a TileGeometry for text.
    static TileGeometryPtr Create(TextStringR textString, TransformCR transform, DRange3dCR range, DgnElementId entityId, TileDisplayParamsCR params, DgnDbR db);
    //! Create a TileGeometry for a part instance.
    static TileGeometryPtr Create(TileGeomPartR part, TransformCR transform, DRange3dCR range, DgnElementId entityId, TileDisplayParamsCR params, DgnDbR db);
    //! Create a TileGeometry for a view attachment
    static TileGeometryPtr Create(Sheet::ViewAttachmentCR, TransformCR, DRange3dCR, TileDisplayParamsCR);

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

    uint32_t _GetExcessiveRefCountThreshold() const override {return 100000;} 

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
    virtual bool _AcceptElement(DgnElementId elementId, TileDisplayParamsCR displayParams) const= 0;
public:
    //! Invoked for each element in the tile's range. Returns false to exclude the element from the tile geometry, or true to include it.
    bool AcceptElement(DgnElementId elementId, TileDisplayParamsCR displayParams) const { return _AcceptElement(elementId, displayParams); }
};

//=======================================================================================
//! Filters elements during collection.
// @bsistruct                                                   Ray.Bentley     09/2017
//=======================================================================================
struct ITileCollectionFilter
{
    virtual bool _AcceptElement(DgnElementId elementId) const = 0;
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   01/18
//=======================================================================================
struct TileElementEntry
{
    using Range = RangeIndex::FBox;

    Range           m_range;
    DgnElementId    m_id;
    double          m_sizeSq;

    TileElementEntry() = default;
    TileElementEntry(DRange3dCR range, DgnElementId id, bool is2d)
        : m_range(range), m_id(id), m_sizeSq(is2d ? range.low.DistanceSquaredXY(range.high) : range.low.DistanceSquared(range.high)) { }

    bool operator<(TileElementEntry const& rhs) const
        {
        if (m_sizeSq != rhs.m_sizeSq)
            return m_sizeSq > rhs.m_sizeSq;
        else
            return m_id < rhs.m_id;
        }
};

using TileElementSet = bset<TileElementEntry>;

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

    DRange3d                        m_range;
    mutable GeometryListMap         m_geometry;
    mutable GeometrySourceMap       m_geometrySources;
    mutable BeMutex                 m_mutex;    // for geometry cache
    mutable BeSQLite::BeDbMutex     m_dbMutex;  // for multi-threaded access to database
    DgnModelPtr                     m_model;
    TileElementSet                  m_elements;
    Options                         m_options;

    friend struct TileGenerator; // Invokes Populate() from ctor
    TileGenerationCache(Options options);
    TileGeneratorStatus Populate(DgnDbR db, DgnModelR model);

    static TileGenerationCachePtr Create(Options options = Options::None) { return new TileGenerationCache(options); }
public:
    DGNPLATFORM_EXPORT ~TileGenerationCache();

    DRange3dCR GetRange() const { return m_range; }
    DgnModelR GetModel() const { return *m_model; }

    bool WantCacheGeometry() const { return Options::CacheGeometryLists == m_options; }
    bool GetCachedGeometry(TileGeometryList& geometry, DgnElementId elementId) const;
    void AddCachedGeometry(DgnElementId elementId, TileGeometryList&& geometry) const;

    BeSQLite::BeDbMutex& GetDbMutex() const { return m_dbMutex; }

    TileElementSet const& GetElements() const { return m_elements; }
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
    DRange3d                m_dgnRange;
    TileNodeList            m_children;
    size_t                  m_depth;
    size_t                  m_siblingIndex;
    double                  m_tolerance;
    TileNodeP               m_parent;
    Transform               m_transformFromDgn;
    mutable DRange3d        m_publishedRange;
    DgnModelCPtr            m_model;
    FeatureAttributesMap    m_attributes;
    bool                    m_isEmpty;
    std::pair<Utf8String, Json::Value> m_tileCustomMetadata;

    TileNode(DgnModelCR model, TransformCR transformFromDgn) : TileNode(model, DRange3d::NullRange(), transformFromDgn, 0, 0, nullptr) { }
    TileNode(DgnModelCR model, DRange3dCR range, TransformCR transformFromDgn, size_t depth, size_t siblingIndex, TileNodeP parent, double tolerance = 0.0)
        : m_dgnRange(range), m_depth(depth), m_siblingIndex(siblingIndex), m_tolerance(tolerance), m_parent(parent), m_transformFromDgn(transformFromDgn), m_publishedRange(DRange3d::NullRange()), m_model(&model), m_isEmpty(true) { }

    virtual PublishableTileGeometry _GeneratePublishableGeometry(DgnDbR dgndb, TileGeometry::NormalMode normalMode=TileGeometry::NormalMode::CurvedSurfacesOnly, bool doSurfacesOnly=false, bool doInstancing=true, ITileGenerationFilterCP filter = nullptr) const = 0;
    virtual TileGeneratorStatus _CollectGeometry(TileGenerationCacheCR cache, DgnDbR db, bool* leafThresholdExceeded, double leafTolerance, double tileTolerance, bool surfacesOnly, size_t leafCountThreshold, ITileCollectionFilterCP filter) { return TileGeneratorStatus::Success; }
    virtual void _ClearGeometry() { }
    virtual WString _GetFileExtension() const = 0;

    uint32_t _GetExcessiveRefCountThreshold() const override {return 0x00ffffff;} // A deep tree can trigger this assertion erroneously.

public:
    DgnModelCR GetModel() const { return *m_model; }
    DRange3dCR GetDgnRange() const { return m_dgnRange; }
    DRange3d GetTileRange() const { DRange3d range = m_dgnRange; m_transformFromDgn.Multiply(range, range); return range; }
    DPoint3d GetTileCenter() const { DRange3d range = GetTileRange(); return DPoint3d::FromInterpolate(range.low, .5, range.high); }
    TransformCR GetTransformFromDgn() const { return m_transformFromDgn; }
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
    DGNPLATFORM_EXPORT void SetPublishedRange(DRange3dCR publishedRange) const;
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

    FeatureAttributesMapCR GetAttributes() const { return m_attributes; }
    FeatureAttributesMapR GetAttributes() { return m_attributes; }
    uint32_t GetAttributesIndex(FeatureAttributesCR attr) { return GetAttributes().GetIndex(attr); }
    uint32_t GetAttributesIndex(TileGeometryCR geometry) { return GetAttributes().GetIndex(geometry.GetAttributes()); }

    TileGeneratorStatus  CollectGeometry(TileGenerationCacheCR cache, DgnDbR db, bool* leafThresholdExceeded, double leafTolerance, double tileTolerance, bool surfacesOnly, size_t leafCountThreshold, ITileCollectionFilterCP filter = nullptr) { return _CollectGeometry(cache, db, leafThresholdExceeded, leafTolerance, tileTolerance, surfacesOnly, leafCountThreshold, filter); }
    void  ClearGeometry() { _ClearGeometry(); }
    WString GetFileExtension() const { return _GetFileExtension(); }
    PublishableTileGeometry GeneratePublishableGeometry(DgnDbR dgndb, TileGeometry::NormalMode normalMode=TileGeometry::NormalMode::CurvedSurfacesOnly, bool doSurfacesOnly=false, bool doInstancing=true, ITileGenerationFilterCP filter = nullptr) const
        { return _GeneratePublishableGeometry(dgndb, normalMode, doSurfacesOnly, doInstancing,  filter); }
    DGNPLATFORM_EXPORT static void ComputeChildTileRanges(bvector<DRange3d>& subTileRanges, DRange3dCR range, size_t splitCount = 3);

    void GetTileCustomMetadata(Utf8StringR name, Json::Value& metadata) const
        {
        name = m_tileCustomMetadata.first;
        metadata = m_tileCustomMetadata.second;
        }

    void ExtractCustomMetadataFrom(TileTree::TileCR inputTile)
        {
        inputTile.GetCustomMetadata(m_tileCustomMetadata.first, m_tileCustomMetadata.second);
        }
    };

//=======================================================================================
//! A TileNode generated from a set of elements.
// @bsistruct                                                   Paul.Connelly   10/16
//=======================================================================================
struct ElementTileNode : TileNode
{
private:
    bool                    m_isLeaf;                         
    mutable bool            m_containsParts;

    TileMeshList GenerateMeshes(DgnDbR db, TileGeometry::NormalMode normalMode, bool doSurfacesOnly, bool doRangeTest, ITileGenerationFilterCP filter, TileGeometryList const& geometries) const;

protected:
    TileGeometryList        m_geometries;

    ElementTileNode(DgnModelCR model, TransformCR transformFromDgn) : TileNode(model, transformFromDgn), m_isLeaf(false), m_containsParts(false) { }
    ElementTileNode(DgnModelCR model, DRange3dCR range, TransformCR transformFromDgn, size_t depth, size_t siblingIndex, TileNodeP parent, double tolerance = 0.0)
        : TileNode(model, range, transformFromDgn, depth, siblingIndex, parent, tolerance), m_isLeaf(false), m_containsParts(false) { }


    DGNPLATFORM_EXPORT PublishableTileGeometry _GeneratePublishableGeometry(DgnDbR, TileGeometry::NormalMode, bool surfacesOnly, bool doInstancing, ITileGenerationFilterCP filter) const override;
    TileGeneratorStatus _CollectGeometry(TileGenerationCacheCR cache, DgnDbR db, bool* leafThresholdExceeded, double leafTolerance, double tileTolerance, bool surfacesOnly, size_t leafCountThreshold, ITileCollectionFilterCP) override;
    void _ClearGeometry() override { m_geometries.clear(); }
    WString _GetFileExtension() const override { return m_containsParts ? L"cmpt" : L"b3dm"; }

public:
    static ElementTileNodePtr Create(DgnModelCR model, TransformCR transformFromDgn) { return new ElementTileNode(model, transformFromDgn); }
    static ElementTileNodePtr Create(DgnModelCR model, DRange3dCR dgnRange, TransformCR transformFromDgn, size_t depth, size_t siblingIndex, TileNodeP parent)
        { return new ElementTileNode(model, dgnRange, transformFromDgn, depth, siblingIndex, parent); }

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
        //! Invoked when a model which exposes a direct URL to a published tileset is processed.
        //! _Begin/_EndProcessModel() will not be invoked. Neither will _AcceptTile() as these models generate no tiles (the tiles already exist elsewhere).
        virtual TileGeneratorStatus _AcceptPublishedTilesetInfo(DgnModelCR model, IGetPublishedTilesetInfoR url) = 0;
        //! Invoked when a model implements its own tileset publisher
        virtual TileGeneratorStatus _AcceptCustomTilesetPublisher(DgnModelCR model, ICustomTilesetPublisherR customPublisher) = 0;
        //! Invoked before a model is processed.
        virtual TileGeneratorStatus _BeginProcessModel(DgnModelCR model) { return TileGeneratorStatus::Success; }
        //! Invoked after a model is processed, with the result of processing.
        virtual TileGeneratorStatus _EndProcessModel(DgnModelCR model, TileNodeP rootTile, TileGeneratorStatus status) { return status; }
        };

    //! Accumulates statistics during tile generation
    struct Statistics
        {
        size_t      m_tileCount = 0;
        double      m_tileGenerationTime = 0.0;
        };
    
    struct GenerateTileResult
        {
        TileNodePtr             m_tile;
        TileGeneratorStatus     m_status;

        explicit GenerateTileResult(TileGeneratorStatus status, TileNodeP tile=nullptr) : m_tile(tile), m_status(status)
            { BeAssert(TileGeneratorStatus::Success != m_status || m_tile.IsValid()); }
        };

    typedef folly::Future<TileGeneratorStatus> FutureStatus;
    typedef folly::Future<GenerateTileResult> FutureGenerateTileResult;
            
        
private:
    Statistics                              m_statistics;
    ITileGenerationProgressMonitorR         m_progressMeter;
    Transform                               m_spatialTransformFromDgn;
    DgnDbR                                  m_dgndb;
    BeAtomic<uint32_t>                      m_totalTiles;
    uint32_t                                m_totalModels;
    BeAtomic<uint32_t>                      m_completedModels;
    ITileCollectionFilterCP                 m_filter;

    struct ElementTileContext
        {
        DgnPlatformLib::Host&   m_host;
        TileGenerationCachePtr  m_cache;
        DgnModelPtr             m_model;
        ITileCollector*         m_collector;
        size_t                  m_maxPointsPerTile;
        double                  m_leafTolerance;
        bool                    m_surfacesOnly;


        ElementTileContext(TileGenerationCacheR cache, DgnModelR model, ITileCollector& collector, double leafTolerance, bool surfacesOnly, size_t maxPointsPerTile)
            : m_host(T_HOST), m_cache(&cache), m_model(&model), m_collector(&collector), m_leafTolerance(leafTolerance), m_maxPointsPerTile(maxPointsPerTile), m_surfacesOnly(surfacesOnly) { }

        };


    FutureGenerateTileResult GenerateElementTiles(ITileCollector& collector, double leafTolerance, bool surfacesOnly, size_t maxPointsPerTile, DgnModelR model);
    FutureStatus PopulateCache(ElementTileContext context);
    FutureGenerateTileResult GenerateTileset(TileGeneratorStatus status, ElementTileContext context);
    FutureGenerateTileResult ProcessParentTile(ElementTileNodePtr parent, ElementTileContext context);
    FutureGenerateTileResult ProcessChildTiles(TileGeneratorStatus status, ElementTileNodePtr parent, ElementTileContext context);
                                                                                
    FutureStatus GenerateTiles(ITileCollector& collector, double leafTolerance, bool surfacesOnly, size_t maxPointsPerTile, DgnModelR model);
    FutureStatus GenerateTilesFromModels(ITileCollector& collector, DgnModelIdSet const& modelIds, double leafTolerance, bool surfacesOnly, size_t maxPointsPerTile);
    FutureStatus GenerateTilesFromTileTree(ITileCollector* collector, double leafTolerance, bool surfacesOnly, GeometricModelP model);

public:
    DGNPLATFORM_EXPORT explicit TileGenerator(DgnDbR dgndb, TransformCR dbToTile, ITileCollectionFilterCP filter=nullptr, ITileGenerationProgressMonitorP progress=nullptr);

    DgnDbR GetDgnDb() const { return m_dgndb; }
    TransformCR GetSpatialTransformFromDgn() const { return m_spatialTransformFromDgn; }
    Transform GetTransformFromDgn(DgnModelCR model) const;

    Statistics const& GetStatistics() const { return m_statistics; }
    ITileGenerationProgressMonitorR GetProgressMeter() { return m_progressMeter; }

    DGNPLATFORM_EXPORT TileGeneratorStatus GenerateTiles(ITileCollector& collector, DgnModelIdSet const& modelIds, double leafTolerance, bool surfacesOnly, size_t maxPointsPerTile);
    DGNPLATFORM_EXPORT static IFacetOptionsPtr CreateTileFacetOptions(double chordTolerance);

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
// Interface for TileTree based models to expose their tree for publishing.
// @bsistruct                                                   Ray.Bentley     08/2016
//=======================================================================================
struct IGetTileTreeForPublishing
{
    // ###TODO: remove this interface when TileGenerator::GenerateTilesFromTileTree can process everything.

    // Override this to disallow publishing.
    virtual bool _AllowPublishing() const { return true; }
};

//=======================================================================================                                                                                                                                              bb
// Describes a published ready-to-use 3D tileset associated with a SpatialModel.
// @bsistruct                                                   Paul.Connelly   08/17
//=======================================================================================
struct PublishedTilesetInfo
{
    Utf8String  m_url;
    DRange3d    m_ecefRange;

    PublishedTilesetInfo() { }
    PublishedTilesetInfo(Utf8StringCR url, DRange3dCR ecefRange) : m_url(url), m_ecefRange(ecefRange) { }

    bool IsValid() const { return !m_url.empty(); }
};

//=======================================================================================
// Interface for models which provide direct URLs to pre-published tilesets.
// @bsistruct                                                   Paul.Connelly   08/17
//=======================================================================================
struct IGetPublishedTilesetInfo
{
    virtual PublishedTilesetInfo _GetPublishedTilesetInfo() = 0;
};

//=======================================================================================
// Interface for models which provide their own tileset publishing methods.
// @bsistruct                                                   Richard.Bois   04/18
//=======================================================================================
struct ICustomTilesetPublisher
    {
    virtual void _DoPublish(BeFileNameCR outDir, WString rootName, DRange3dR range, const Transform& tileToECEF) = 0;
    };

#define COMPARE_VALUES_TOLERANCE(val0, val1, tol) { if (val0 < val1 - tol) return true; if (val0 > val1 + tol) return false; }
#define COMPARE_VALUES(val0, val1) { if (val0 < val1) return true; if (val0 > val1) return false; }

// static utility methods
// @bsistruct                                                   Ray.Bentley     08/2016
//=======================================================================================
struct TileUtil
{
    DGNPLATFORM_EXPORT static BentleyStatus WriteJsonToFile (WCharCP fileName, Json::Value const& value);
    DGNPLATFORM_EXPORT static BentleyStatus ReadJsonFromFile (Json::Value& value, WCharCP fileName);
    DGNPLATFORM_EXPORT static WString GetRootNameForModel(DgnModelId modelId, bool asClassifier);

    struct PointComparator
        {
        double  m_tolerance;

        explicit PointComparator(double tolerance) : m_tolerance(tolerance) { }


        bool operator()(DPoint3dCR lhs, DPoint3dCR rhs) const
            {
            COMPARE_VALUES_TOLERANCE(lhs.x, rhs.x, m_tolerance);
            COMPARE_VALUES_TOLERANCE(lhs.y, rhs.y, m_tolerance);
            COMPARE_VALUES_TOLERANCE(lhs.z, rhs.z, m_tolerance);
                                                                    
            return false;
            }
        };

};  // TileUtil

END_BENTLEY_RENDER_NAMESPACE


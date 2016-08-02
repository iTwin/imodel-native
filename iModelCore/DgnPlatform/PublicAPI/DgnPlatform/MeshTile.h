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

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
class XYZRangeTreeRoot;
END_BENTLEY_GEOMETRY_NAMESPACE

BENTLEY_RENDER_TYPEDEFS(TileGeometryCache);
BENTLEY_RENDER_TYPEDEFS(Triangle);
BENTLEY_RENDER_TYPEDEFS(TileMesh);
BENTLEY_RENDER_TYPEDEFS(TileMeshBuilder);
BENTLEY_RENDER_TYPEDEFS(TileNode);
BENTLEY_RENDER_TYPEDEFS(TileGenerator);
BENTLEY_RENDER_TYPEDEFS(TileGeometry);

BENTLEY_RENDER_REF_COUNTED_PTR(TileMesh);
BENTLEY_RENDER_REF_COUNTED_PTR(TileMeshBuilder);
BENTLEY_RENDER_REF_COUNTED_PTR(TileGeometry);

BEGIN_BENTLEY_RENDER_NAMESPACE

typedef bvector<TileMeshPtr> TileMeshList;
typedef bvector<TileNode> TileNodeList;
typedef bvector<TileNodeP> TileNodePList;
typedef bvector<TileGeometryPtr> TileGeometryList;

//=======================================================================================
//! Holds geometry processed during tile generation. Objects produced during this process
//! may holds pointers into the cache; they become invalid once the cache itself is
//! destroyed.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct TileGeometryCache
{
    struct TextureImage : RefCountedBase
    {
    private:
        ByteStream  m_image;
        Point2d     m_size;
        size_t      m_id;

        static RefCountedPtr<TextureImage> Create(Point2dCR size, ByteStream&& image, size_t id) { return new TextureImage(size, std::move(image), id); }
    private:
        TextureImage(Point2dCR size, ByteStream&& image, size_t id) : m_size(size), m_id(id), m_image(std::move(image)) { }

        ByteStream const& GetImage() const { return m_image; }
        Point2d GetSize() const { return m_size; }
        size_t GetId() const { return m_id; }
    };
private:
    struct TextureImageKey
    {
        ColorDef        m_color;
        MaterialCP      m_material;

        TextureImageKey() : m_material(nullptr) { }
        TextureImageKey(GraphicParamsCR params) : m_color(params.GetFillColor()), m_material(params.GetMaterial()) { }

        bool operator<(TextureImageKey const& rhs) const
            {
            if (rhs.m_material != m_material)
                return m_material < rhs.m_material; // ###TODO: Only if texture - and texture may blend color....
            else
                return m_color.GetValue() < rhs.m_color.GetValue();
            }
    };

    typedef RefCountedPtr<TextureImage> TextureImagePtr;
    typedef bmap<TextureImageKey, TextureImagePtr> TextureImageMap;

    XYZRangeTreeRoot*       m_tree;
    TextureImageMap         m_textures;
    Transform               m_transformToDgn;
    Transform               m_transformFromDgn;
    size_t                  m_nextTextureId;
public:
    DGNPLATFORM_EXPORT TileGeometryCache(TransformCR transformFromDgn);
    DGNPLATFORM_EXPORT ~TileGeometryCache();

    XYZRangeTreeRoot& GetTree() const { return *m_tree; }
    DGNPLATFORM_EXPORT DRange3d GetRange() const;
    TransformCR GetTransformToDgn() const { return m_transformToDgn; }

    void ResolveTexture(GraphicParamsCR params);
    DGNPLATFORM_EXPORT TextureImage const* GetTextureImage(GraphicParamsCR params) const;
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
//! Represents a single mesh of uniform symbology within a TileNode, consisting of
//! vertex/normal/uv-param/elementID arrays indexed by an array of triangles.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct TileMesh : RefCountedBase
{
private:
    GraphicParamsCP         m_graphicParams;   // pointer into TileGeometryCache
    bvector<Triangle>       m_triangles;
    bvector<DPoint3d>       m_points;
    bvector<DVec3d>         m_normals;
    bvector<DPoint2d>       m_uvParams;
    bvector<DgnElementId>   m_elementIds;   // invalid IDs for clutter geometry

    explicit TileMesh(GraphicParamsCP params) : m_graphicParams(params) { }

    template<typename T> T const* GetMember(bvector<T> const& from, uint32_t at) const { return at < from.size() ? &from[at] : nullptr; }
public:
    static TileMeshPtr Create(GraphicParamsCP params) { return new TileMesh(params); }

    DGNPLATFORM_EXPORT DRange3d GetTriangleRange(TriangleCR triangle) const;
    DGNPLATFORM_EXPORT DVec3d GetTriangleNormal(TriangleCR triangle) const;
    DGNPLATFORM_EXPORT bool HasNonPlanarNormals() const;

    GraphicParamsCP GetGraphicParams() const { return m_graphicParams; } //!< The mesh symbology
    bvector<Triangle> const& Triangles() const { return m_triangles; } //!< Triangles defined as a set of 3 indices into the vertex attribute arrays.
    bvector<DPoint3d> const& Points() const { return m_points; } //!< Position vertex attribute array
    bvector<DVec3d> const& Normals() const { return m_normals; } //!< Normal vertex attribute array
    bvector<DPoint2d> const& Params() const { return m_uvParams; } //!< UV params vertex attribute array
    bvector<DgnElementId> const& ElementIds() const { return m_elementIds; } //!< Vertex attribute array specifying the ID of the element from which the vertex was produced

    TriangleCP GetTriangle(uint32_t index) const { return GetMember(m_triangles, index); }
    DPoint3dCP GetPoint(uint32_t index) const { return GetMember(m_points, index); }
    DVec3dCP GetNormal(uint32_t index) const { return GetMember(m_normals, index); }
    DPoint2dCP GetParam(uint32_t index) const { return GetMember(m_uvParams, index); }
    DgnElementId GetElementId(uint32_t index) const { auto pId = GetMember(m_elementIds, index); return nullptr != pId ? *pId : DgnElementId(); }
    bool IsEmpty() const { return m_triangles.empty(); }

    void AddTriangle(TriangleCR triangle) { m_triangles.push_back(triangle); }
    uint32_t AddVertex(DPoint3dCR point, DVec3dCP normal, DPoint2dCP param, DgnElementId elemId);
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
        DgnElementId    m_elementId;
        bool            m_normalValid = false;
        bool            m_paramValid = false;

        VertexKey() { }
        VertexKey(DPoint3dCR point, DVec3dCP normal, DPoint2dCP param, DgnElementId elemId) : m_point(point), m_normalValid(nullptr != normal), m_paramValid(nullptr != param), m_elementId(elemId)
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

    Transform           m_transformToDgn;
    TileMeshPtr         m_mesh;
    VertexMap           m_vertexMap;
    TriangleSet         m_triangleSet;
    double              m_tolerance;
    size_t              m_triangleIndex;

    TileMeshBuilder(GraphicParamsCP params, TransformCP transformToDgn, double tolerance) : m_mesh(TileMesh::Create(params)), m_vertexMap(VertexKey::Comparator(tolerance)),
            m_transformToDgn(nullptr != transformToDgn ? *transformToDgn : Transform::FromIdentity()), m_tolerance(tolerance), m_triangleIndex(0) { }
public:
    static TileMeshBuilderPtr Create(GraphicParamsCP params, TransformCP transformToDgn, double tolerance) { return new TileMeshBuilder(params, transformToDgn, tolerance); }

    void AddTriangle(PolyfaceVisitorR visitor, DgnElementId elemId, bool doVertexClustering, bool duplicateTwoSidedTriangles);
    void AddTriangle(TriangleCR triangle, TileMeshCR mesh);
    void AddTriangle(TriangleCR triangle);
    uint32_t AddClusteredVertex(VertexKey const& vertex);
    uint32_t AddVertex(VertexKey const& vertex);

    TileMeshP GetMesh() { return m_mesh.get(); } //!< The mesh under construction
};

//=======================================================================================
//! Representation of geometry processed by a TileGenerator, consisting of an IGeometry,
//! an ISolidKernelEntity, or nothing.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct TileGeometry : RefCountedBase
{
    enum class Type
    {
        Solid,      //!< This TileGeometry contains an ISolidKernelEntity
        Geometry,   //!< This TileGeometry contains an IGeometry
        Empty,      //!< This TileGeometry contains no geometry
    };

    enum class NormalMode
    {
        Never,              //!< Never generate normals
        Always,             //!< Always generate normals
        CurvedSurfacesOnly, //!< Generate normals only for curved surfaces
    };
private:
    typedef bmap<double, PolyfaceHeaderPtr> Tesselations;

    union
        {
        ISolidKernelEntityP m_solidEntity;
        IGeometryP          m_geometry;
        };
    GraphicParams           m_params;
    Transform               m_transform;
    Tesselations            m_tesselations;
    DRange3d                m_range;
    DgnElementId            m_elementId;
    size_t                  m_facetCount;
    double                  m_facetCountDensity;
    Type                    m_type;
    bool                    m_isCurved;
    bool                    m_isInstanced; // ###TODO: unused...?

    TileGeometry(TransformCR tf, DRange3dCR range, DgnElementId elemId, GraphicParamsCR params, bool isCurved);

    void Init(IGeometryR geometry, IFacetOptionsR facetOptions);
    void Init(ISolidKernelEntityR solid, IFacetOptionsR facetOptions);

    IFacetOptionsPtr CreateFacetOptions(double chordTolerance, NormalMode normalMode) const;
public:
    ~TileGeometry();

    //! Create a TileGeometry for an IGeometry
    static TileGeometryPtr Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, GraphicParamsCR params, IFacetOptionsR facetOptions, bool isCurved);
    //! Create a TileGeometry for an ISolidKernelEntity
    static TileGeometryPtr Create(ISolidKernelEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, GraphicParamsCR params, IFacetOptionsR facetOptions);

    Type GetType() const { return m_type; } //!< The type of geometry contained within
    ISolidKernelEntityP GetSolidEntity() const { return Type::Solid == GetType() ? m_solidEntity : nullptr; } //!< The contained ISolidKernelEntity, if any
    IGeometryP GetGeometry() const { return Type::Geometry == GetType() ? m_geometry : nullptr; } //!< The contained IGeometry, if any

    GraphicParamsCR GetGraphicParams() const { return m_params; }
    TransformCR GetTransform() const { return m_transform; }
    DRange3dCR GetRange() const { return m_range; }
    DgnElementId GetElementId() const { return m_elementId; } //!< The ID of the element from which this geometry was produced
    size_t GetFacetCount() const { return m_facetCount; }
    double GetFacetCountDensity() const { return m_facetCountDensity; }
    bool IsCurved() const { return m_isCurved; }

    bool HasTexture() const;
    PolyfaceHeaderPtr GetPolyface(double chordTolerance, NormalMode normalMode);
};

//=======================================================================================
//! Represents one tile in a HLOD tree occupying a given range and containing higher-LOD
//! child tiles within the same range.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct TileNode
{
private:
    DRange3d            m_range;
    TileNodeList        m_children;
    size_t              m_depth;
    size_t              m_siblingIndex;
    double              m_tolerance;
    TileNodeP           m_parent;

    static void ComputeSubTiles(bvector<DRange3d>& subTileRanges, TileGeometryCacheR geometryCache, DRange3dCR range, size_t maxFacetsPerSubTile);

    TileMeshPtr GetRangeMesh(DRange3dCR range, TileGeometryCacheR geometryCache) const;
public:
    TileNode() : TileNode(DRange3d::NullRange(), 0, 0, 0.0, nullptr) { }
    TileNode(DRange3dCR range, size_t depth, size_t siblingIndex, double tolerance, TileNodeP parent)
        : m_range(range), m_depth(depth), m_siblingIndex(siblingIndex), m_tolerance(tolerance), m_parent(parent) { }

    DRange3dCR GetRange() const { return m_range; }
    TileNodeList const& GetChildren() const { return m_children; }
    size_t GetDepth() const { return m_depth; } //!< This node's depth from the root tile node
    size_t GetSiblingIndex() const { return m_siblingIndex; } //!< This node's order within its siblings at the same depth
    double GetTolerance() const { return m_tolerance; }
    TileNodeCP GetParent() const { return m_parent; } //!< The direct parent of this node
    TileNodeP GetParent() { return m_parent; } //!< The direct parent of this node

    DGNPLATFORM_EXPORT void ComputeTiles(TileGeometryCacheR geometryCache, double chordTolerance, size_t maxPointsPerTile);
    DGNPLATFORM_EXPORT double GetMaxDiameter(double tolerance) const;

    //! Generate a list of meshes from this tile's geometry.
    DGNPLATFORM_EXPORT TileMeshList GenerateMeshes(TileGeometryCacheR geometryCache, double tolerance, TileGeometry::NormalMode normalMode=TileGeometry::NormalMode::CurvedSurfacesOnly, bool twoSidedTriangles=false) const;
    DGNPLATFORM_EXPORT TileMeshPtr GetDefaultMesh(TileGeometryCacheR geometryCache) const;
    DGNPLATFORM_EXPORT TileMeshPtr GetRangeMesh(TileGeometryCacheR geometryCache) const;
    DGNPLATFORM_EXPORT size_t GetNodeCount() const;
    DGNPLATFORM_EXPORT size_t GetMaxDepth() const;
    DGNPLATFORM_EXPORT void GetTiles(TileNodePList& tiles);
    DGNPLATFORM_EXPORT TileNodePList GetTiles();
};

//=======================================================================================
//! Generates a HLOD tree of TileNodes from a set of elements.
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

    enum class TaskName
    {
        CollectingGeometry,
        CreatingTiles,
    };

    //! Interface adopted by an object which collects generated tiles
    struct EXPORT_VTABLE_ATTRIBUTE ITileCollector
    {
        //! Invoked from one of several worker threads for each generated tile.
        virtual Status _AcceptTile(TileNodeCR tileNode) = 0;
    };

    //! Interface adopted by an object which tracks progress of the tile generation process
    struct EXPORT_VTABLE_ATTRIBUTE IProgressMeter
    {
        virtual void _IndicateProgress(double fraction) { } //!< Invoked to announce the current % completion
        virtual bool _WasAborted() { return false; } //!< Return true to abort tile generation
        virtual void _SetTaskName(TaskName taskName) { } //!< Invoked to announce the current task
    };

    //! Accumulates statistics during tile generation
    struct Statistics
    {
        size_t      m_tileCount = 0;
        size_t      m_tileSize = 0;
        size_t      m_tileDepth = 0;
        size_t      m_emptyNodeCount = 0;
        double      m_collectionTime = 0.0;
        double      m_tileCreationTime = 0.0;
    };
private:
    Statistics          m_statistics;
    TileGeometryCache   m_geometryCache;
    IProgressMeter&     m_progressMeter;

    static void ComputeSubRanges(bvector<DRange3d>& subRanges, bvector<DPoint3d> const& points, size_t maxPoints, DRange3dCR range);
public:
    DGNPLATFORM_EXPORT explicit TileGenerator(TransformCR transformFromDgn, IProgressMeter* progressMeter=nullptr);

    //! Populates the TileGeometryCache from the contents of the specified view
    DGNPLATFORM_EXPORT Status LoadGeometry(ViewControllerR view, double toleranceInMeters);
    //! Generates the HLOD tree from the contents of the TileGeometryCache within the specified range
    DGNPLATFORM_EXPORT Status GenerateTiles(TileNodeR rootTile, DRange3dCR range, double leafTolerance, size_t maxPointsPerTile=30000);
    DGNPLATFORM_EXPORT Status CollectTiles(TileNodeR rootTile, ITileCollector& collector);
    DGNPLATFORM_EXPORT static void SplitMeshToMaximumSize(TileMeshList& meshes, TileMeshR mesh, size_t maxPoints);

    Statistics const& GetStatistics() const { return m_statistics; }
};

//=======================================================================================
// FacetCountUtil is a wrapper structure that contains helper methods to approximate
// the amount of facets a solid, curve vector or BRep will contain after processing 
// when using specific facet options
//
// @bsistruct                                                   Diego.Pinate    07/16
//=======================================================================================
struct FacetCountUtil
{
    //! Returns an approximation of the facet counts of a solid primitive given the facet options
    //! @param [in] solidPrimitive The ISolidPrimitive to be inspected
    //! @param [in] facetOptions The facet options: this takes into account chord tolerance and angle tolerance
    static DGNPLATFORM_EXPORT size_t GetFacetCount (ISolidPrimitiveCR solidPrimitive, IFacetOptionsR facetOptions);

    //! Returns an approximation of the facet counts of a curve vector given the facet options
    //! @param [in] curveVector The CurveVector to be inspected
    //! @param [in] facetOptions The facet options: this takes into account chord tolerance and angle tolerance
    //! @return An approximation of the number of strokes/facets that will be generated from the curve vector
    static DGNPLATFORM_EXPORT size_t GetFacetCount (CurveVectorCR curveVector, IFacetOptionsR facetOptions);

    //! Returns an approximation of the facet counts of a bspline surface given the facet options
    //! @param [in] surface Bspline surface to be inspected
    //! @param [in] facetOptions The facet options: this takes into account chord tolerance and angle tolerance
    //! @param [in] useMax if true, the algorithm calculates the approximation using the max amount of possible facets instead of the average
    //! @return An approximation of the number of strokes/facets that will be generated from the surface
    static DGNPLATFORM_EXPORT size_t GetFacetCount (MSBsplineSurfaceCR surface, IFacetOptionsR facetOptions, bool useMax = false);

    //! Returns an approximation of the facet counts of an IGeometry the facet options
    //! @param [in] geometry Geometry to be inspected
    //! @param [in] facetOptions The facet options: this takes into account chord tolerance and angle tolerance
    //! @param [in] useMax if true, the algorithm calculates the approximation using the max amount of possible facets instead of the average
    //! @return An approximation of the number of strokes/facets that will be generated from the surface
    static DGNPLATFORM_EXPORT size_t GetFacetCount (IGeometryCR geometry, IFacetOptionsR facetOptions);

};

END_BENTLEY_RENDER_NAMESPACE


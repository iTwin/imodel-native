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

BENTLEY_RENDER_TYPEDEFS(Triangle);
BENTLEY_RENDER_TYPEDEFS(TilePolyline);
BENTLEY_RENDER_TYPEDEFS(TileMesh);
BENTLEY_RENDER_TYPEDEFS(TileMeshBuilder);
BENTLEY_RENDER_TYPEDEFS(TileNode);
BENTLEY_RENDER_TYPEDEFS(TileGenerator);
BENTLEY_RENDER_TYPEDEFS(TileGeometry);
BENTLEY_RENDER_TYPEDEFS(TileDisplayParams);
BENTLEY_RENDER_TYPEDEFS(TileTextureImage);

BENTLEY_RENDER_REF_COUNTED_PTR(TileMesh);
BENTLEY_RENDER_REF_COUNTED_PTR(TileNode);
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
// ! Holds a texture image.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct TileTextureImage : RefCountedBase
    {
    private:
        ImageSource       m_imageSource;

        static ImageSource Load(TileDisplayParamsCR params, DgnDbR db);

        TileTextureImage(ImageSource&& imageSource) : m_imageSource(std::move(imageSource)) { BeAssert(m_imageSource.IsValid()); }
        TileTextureImage(ImageSource& imageSource) : m_imageSource (imageSource) { BeAssert(m_imageSource.IsValid()); }
    public:
        static TileTextureImagePtr Create(ImageSource&& imageSource) { return new TileTextureImage(std::move(imageSource)); }
        static TileTextureImagePtr Create(ImageSource& imageSource) { return new TileTextureImage(imageSource); }

        ImageSourceCR GetImageSource() const { return m_imageSource; }
        static void ResolveTexture(TileDisplayParamsR params, DgnDbR db);
    };

//=======================================================================================
//! Display params associated with TileGeometry. Based on GraphicParams and GeometryParams.
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct TileDisplayParams : RefCountedBase
{
private:
    uint32_t                m_fillColor;
    DgnMaterialId           m_materialId;
    TileTextureImagePtr     m_textureImage;
    bool                    m_ignoreLighting;

    TileDisplayParams(GraphicParamsCP graphicParams, GeometryParamsCP geometryParams);
    TileDisplayParams(uint32_t fillColor, TileTextureImageP texture, bool ignoreLighting) : m_fillColor(fillColor), m_textureImage(texture), m_ignoreLighting(ignoreLighting) { }
public:
    static TileDisplayParamsPtr Create() { return Create(nullptr, nullptr); }
    static TileDisplayParamsPtr Create(GraphicParamsCR graphicParams, GeometryParamsCR geometryParams) { return Create(&graphicParams, &geometryParams); }
    static TileDisplayParamsPtr Create(GraphicParamsCP graphicParams, GeometryParamsCP geometryParams) { return new TileDisplayParams(graphicParams, geometryParams); }
    static TileDisplayParamsPtr Create(uint32_t fillColor, TileTextureImageP textureImage, bool ignoreLighting) { return new TileDisplayParams(fillColor, textureImage, ignoreLighting); }

    bool operator<(TileDisplayParams const& rhs) const;

    DgnMaterialId GetMaterialId() const { return m_materialId; }
    uint32_t GetFillColor() const { return m_fillColor; }
    bool GetIgnoreLighting() const { return m_ignoreLighting; }
    DgnTextureCPtr QueryTexture(DgnDbR db) const;
    TileTextureImagePtr& TextureImage() { return m_textureImage; }
    TileTextureImageCP GetTextureImage() const { return m_textureImage.get(); }
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
    bvector<DgnElementId>   m_elementIds;   // invalid IDs for clutter geometry
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
    bvector<DgnElementId> const& ElementIds() const { return m_elementIds; } //!< Vertex attribute array specifying the ID of the element from which the vertex was produced

    TriangleCP GetTriangle(uint32_t index) const { return GetMember(m_triangles, index); }
    DPoint3dCP GetPoint(uint32_t index) const { return GetMember(m_points, index); }
    DVec3dCP GetNormal(uint32_t index) const { return GetMember(m_normals, index); }
    DPoint2dCP GetParam(uint32_t index) const { return GetMember(m_uvParams, index); }
    DgnElementId GetElementId(uint32_t index) const { auto pId = GetMember(m_elementIds, index); return nullptr != pId ? *pId : DgnElementId(); }
    bool IsEmpty() const { return m_triangles.empty() && m_polylines.empty(); }
    bool ValidIdsPresent() const { return m_validIdsPresent; }

    void AddTriangle(TriangleCR triangle) { m_triangles.push_back(triangle); }
    void AddPolyline (TilePolyline polyline) { m_polylines.push_back(polyline); }
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

    TileMeshBuilder(TileDisplayParamsPtr& params, TransformCP transformToDgn, double tolerance) : m_mesh(TileMesh::Create(params)), m_vertexMap(VertexKey::Comparator(tolerance)),
            m_transformToDgn(nullptr != transformToDgn ? *transformToDgn : Transform::FromIdentity()), m_tolerance(tolerance), m_triangleIndex(0) { }
public:
    static TileMeshBuilderPtr Create(TileDisplayParamsPtr& params, TransformCP transformToDgn, double tolerance) { return new TileMeshBuilder(params, transformToDgn, tolerance); }

    DGNPLATFORM_EXPORT void AddTriangle(PolyfaceVisitorR visitor, DgnElementId elemId, bool doVertexClustering, bool duplicateTwoSidedTriangles);
    DGNPLATFORM_EXPORT void AddPolyline (bvector<DPoint3d>const& polyline, DgnElementId elemId, bool doVertexClustering);

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
private:
    TileDisplayParamsPtr    m_params;
    Transform               m_transform;
    DRange3d                m_range;
    DgnElementId            m_elementId;
    size_t                  m_facetCount;
    double                  m_facetCountDensity;
    bool                    m_isCurved;
    bool                    m_hasTexture;

protected:
    TileGeometry(TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsPtr& params, bool isCurved, DgnDbR db);

    virtual PolyfaceHeaderPtr _GetPolyface(IFacetOptionsR facetOptions) = 0;
    virtual CurveVectorPtr _GetStrokedCurve(double chordTolerance) = 0;

    void SetFacetCount(size_t numFacets);
    IFacetOptionsPtr CreateFacetOptions(double chordTolerance, NormalMode normalMode) const;
public:
    TileDisplayParamsPtr GetDisplayParams() const { return m_params; }
    TransformCR GetTransform() const { return m_transform; }
    DRange3dCR GetRange() const { return m_range; }
    DgnElementId GetElementId() const { return m_elementId; } //!< The ID of the element from which this geometry was produced

    size_t GetFacetCount() const { return m_facetCount; }
    double GetFacetCountDensity() const { return m_facetCountDensity; }

    bool IsCurved() const { return m_isCurved; }
    bool HasTexture() const { return m_hasTexture; }

    PolyfaceHeaderPtr GetPolyface(double chordTolerance, NormalMode normalMode);
    CurveVectorPtr    GetStrokedCurve (double chordTolerance) { return _GetStrokedCurve(chordTolerance); }
    
    //! Create a TileGeometry for an IGeometry
    static TileGeometryPtr Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsPtr& params, IFacetOptionsR facetOptions, bool isCurved, DgnDbR db);
    //! Create a TileGeometry for an ISolidKernelEntity
    static TileGeometryPtr Create(ISolidKernelEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsPtr& params, IFacetOptionsR facetOptions, DgnDbR db);
};

//=======================================================================================
//! Represents one tile in a HLOD tree occupying a given range and containing higher-LOD
//! child tiles within the same range.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct TileNode : RefCountedBase
{
private:
    DRange3d            m_range;
    TileNodeList        m_children;
    size_t              m_depth;
    size_t              m_siblingIndex;
    double              m_tolerance;
    TileNodeP           m_parent;
    WString             m_subdirectory;

protected:
    TileNode() : TileNode(DRange3d::NullRange(), 0, 0, 0.0, nullptr) { }
    TileNode(DRange3dCR range, size_t depth, size_t siblingIndex, double tolerance, TileNodeP parent)
        : m_range(range), m_depth(depth), m_siblingIndex(siblingIndex), m_tolerance(tolerance), m_parent(parent) { }

public:
    static TileNodePtr Create() { return new TileNode(); }
    static TileNodePtr Create(DRange3dCR range, size_t depth, size_t siblingIndex, double tolerance, TileNodeP parent)
        { return new TileNode(range, depth, siblingIndex, tolerance, parent); }

    DRange3dCR GetRange() const { return m_range; }
    DPoint3d   GetCenter() const { return DPoint3d::FromInterpolate (m_range.low, .5, m_range.high); }
    size_t GetDepth() const { return m_depth; } //!< This node's depth from the root tile node
    size_t GetSiblingIndex() const { return m_siblingIndex; } //!< This node's order within its siblings at the same depth
    double GetTolerance() const { return m_tolerance; }

    TileNodeCP GetParent() const { return m_parent; } //!< The direct parent of this node
    TileNodeP GetParent() { return m_parent; } //!< The direct parent of this node
    TileNodeList const& GetChildren() const { return m_children; } //!< The direct children of this node
    TileNodeList& GetChildren() { return m_children; } //!< The direct children of this node
    WStringCR GetSubdirectory() const { return m_subdirectory; }
    void SetSubdirectory (WStringCR subdirectory) { m_subdirectory = subdirectory; }
    void SetRange (DRange3dCR range) { m_range = range; }

    DGNPLATFORM_EXPORT double GetMaxDiameter(double tolerance) const;

    //! Generate a list of meshes from this tile's geometry.
    DGNPLATFORM_EXPORT size_t GetNodeCount() const;
    DGNPLATFORM_EXPORT size_t GetMaxDepth() const;
    DGNPLATFORM_EXPORT void GetTiles(TileNodePList& tiles);
    DGNPLATFORM_EXPORT TileNodePList GetTiles();
    DGNPLATFORM_EXPORT WString GetNameSuffix() const;
    DGNPLATFORM_EXPORT BeFileNameStatus GenerateSubdirectories (size_t maxTilesPerDirectory, BeFileNameCR dataDirectory);
    DGNPLATFORM_EXPORT WString GetRelativePath (WCharCP rootName, WCharCP extension) const;

    DGNPLATFORM_EXPORT void ComputeTiles(double chordTolerance, size_t maxPointsPerTile, BeSQLite::VirtualSet const& vset, DgnDbR db);
    DGNPLATFORM_EXPORT static void ComputeSubTiles(bvector<DRange3d>& subTileRanges, DRange3dCR range, size_t maxPointsPerSubTile, BeSQLite::VirtualSet const& vset, DgnDbR db);
    DGNPLATFORM_EXPORT virtual TileMeshList _GenerateMeshes(ViewControllerCR view, TileGeometry::NormalMode normalMode=TileGeometry::NormalMode::CurvedSurfacesOnly, bool twoSidedTriangles=false) const;
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
        NotImplemented,
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
        virtual void _IndicateProgress(uint32_t completed, uint32_t total) { } //!< Invoked to announce the current ratio completed
        virtual bool _WasAborted() { return false; } //!< Return true to abort tile generation
        virtual void _SetTaskName(TaskName taskName) { } //!< Invoked to announce the current task
        virtual void _SetModel (DgnModelCP dgnModel) { }
    };

    //! Accumulates statistics during tile generation
    struct Statistics
    {
        size_t      m_tileCount = 0;
        size_t      m_tileDepth = 0;
        double      m_collectionTime = 0.0;
        double      m_tileCreationTime = 0.0;
    };
private:
    Statistics          m_statistics;
    IProgressMeter&     m_progressMeter;

    static void ComputeSubRanges(bvector<DRange3d>& subRanges, bvector<DPoint3d> const& points, size_t maxPoints, DRange3dCR range);
public:
    DGNPLATFORM_EXPORT explicit TileGenerator(TransformCR transformFromDgn, IProgressMeter* progressMeter=nullptr);

    DGNPLATFORM_EXPORT Status CollectTiles(TileNodeR rootTile, ITileCollector& collector);
    DGNPLATFORM_EXPORT static void SplitMeshToMaximumSize(TileMeshList& meshes, TileMeshR mesh, size_t maxPoints);

    Statistics const& GetStatistics() const { return m_statistics; }

    DGNPLATFORM_EXPORT Status GenerateTiles(TileNodePtr& root, ViewControllerR view, size_t maxPointsPerTile);
};

//=======================================================================================
// Interface for models to generate HLOD tree of TileNodes 
// @bsistruct                                                   Ray.Bentley     08/2016
//=======================================================================================
struct IGenerateMeshTiles
{
    virtual TileGenerator::Status _GenerateMeshTiles(TileNodePtr& rootTile, TransformCR transformDbToTile) = 0;

};  // IPublishModelMeshTiles

END_BENTLEY_RENDER_NAMESPACE


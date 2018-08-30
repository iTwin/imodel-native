/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Tile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/Tile.h>
#include <numeric>
#include <DgnPlatform/RangeIndex.h>
#include <GeomJsonWireFormat/JsonUtils.h>
#include <numeric>
#include <inttypes.h>
#if defined (BENTLEYCONFIG_PARASOLID) 
#include <DgnPlatform/DgnBRep/PSolidUtil.h>
#endif

USING_NAMESPACE_TILE
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES

// Obsolete versions of table storing tile data
#define TABLE_NAME_TileTree3 "TileTree3"

// 4th version of this table: modified for iModelJs (imodel02 branch):
// Element tiles are now the only types of tiles produced and cached by the backend.
//  - Remove ContentType and Expires columns
//  - Add Metadata column, containing data formerly stored in the binary stream (geometry flags like 'is curved', zoom factor, etc)
#define TABLE_NAME_TileTree "TileTree4"

// Second version: Same primary key as tile data table.
#define TABLE_NAME_TileTreeCreateTime "TileTreeCreateTime2"

#define COLUMN_TileTree_TileId TABLE_NAME_TileTree ".TileId"
#define COLUMN_TileTreeCreateTime_TileId TABLE_NAME_TileTreeCreateTime ".TileId"
#define JOIN_TileTreeTables TABLE_NAME_TileTree " JOIN " TABLE_NAME_TileTreeCreateTime " ON " COLUMN_TileTree_TileId "=" COLUMN_TileTreeCreateTime_TileId

BEGIN_UNNAMED_NAMESPACE

using FBox3d = RangeIndex::FBox;

constexpr double s_minRangeBoxSize = 2.5;     // Threshold below which we consider geometry/element too small to contribute to tile mesh ###TODO: Revisit...
constexpr double s_tileScreenSize = 512.0;
constexpr double s_minToleranceRatioMultiplier = 2.0;
constexpr double s_minToleranceRatio = s_tileScreenSize * s_minToleranceRatioMultiplier;
constexpr uint32_t s_minElementsPerTile = 100; // ###TODO: The complexity of a single element's geometry can vary wildly...
constexpr uint32_t s_hardMaxFeaturesPerTile = 2048*1024;
constexpr double s_maxLeafTolerance = 1.0; // the maximum tolerance at which we will stop subdividing tiles, regardless of # of elements contained or whether curved geometry exists.

#if defined (BENTLEYCONFIG_PARASOLID) 
static RefCountedPtr<PSolidThreadUtil::MainThreadMark> s_psolidMainThreadMark;
#endif

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/15
//=======================================================================================
struct CacheBlobHeader
{
    enum {DB_Signature06 = 0x0600};
    uint32_t m_signature;    // write this so we can detect errors on read
    uint32_t m_size;

    CacheBlobHeader(uint32_t size) {m_signature = DB_Signature06; m_size=size;}
    CacheBlobHeader(SnappyReader& in) {uint32_t actuallyRead; in._Read((Byte*) this, sizeof(*this), actuallyRead);}
};

DEFINE_POINTER_SUFFIX_TYPEDEFS(ClassificationTree);
DEFINE_POINTER_SUFFIX_TYPEDEFS(GeometryLoader);
DEFINE_POINTER_SUFFIX_TYPEDEFS(TileContext);
DEFINE_POINTER_SUFFIX_TYPEDEFS(MeshGenerator);

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     08/18
//=======================================================================================
struct ClassificationLoader : Loader
{
    DEFINE_T_SUPER(Loader);
private:
    double m_offset; // NB: will be used in future?
    DRange3d m_range;

    bool SeparatePrimitivesById() const final { return true; }
    CurveVectorPtr Preprocess(CurveVectorR cv) const final { return &cv; }
    PolyfaceHeaderPtr Preprocess(PolyfaceHeaderR pf) const final;
public:
    ClassificationLoader(ClassificationTree& tree, ContentIdCR contentId);
};

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     08/18
//=======================================================================================
struct ClassificationTree : Tree
{
    DEFINE_T_SUPER(Tree);

    double m_classifierOffset; // NB: will be initialized+used in future?
    DRange3d m_classifierRange;

    ClassificationTree(GeometricModelCR model, TransformCR location, DRange3dCR range, Render::SystemR system)
        : T_Super(model, location, range, system)
        {
        Transform transformFromDgn;
        transformFromDgn.InverseOf(location);
        transformFromDgn.Multiply(m_classifierRange, model.GetDgnDb().GeoLocation().GetProjectExtents());
        }

    Utf8String _GetId() const final { Utf8String id("Classifier_"); id.append(T_Super::_GetId()); return id; }
    LoaderPtr CreateLoader(ContentIdCR contentId) final { return new ClassificationLoader(*this, contentId); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/18
//=======================================================================================
struct GeometryLoader
{
private:
    LoaderCR m_loader;
    DRange3d m_range;
    double m_tolerance;
    Render::Primitives::GeometryCollection m_geometry;
    DRange3d m_contentRange = DRange3d::NullRange();
public:
    explicit GeometryLoader(LoaderCR loader);

    TreeR GetTree() const { return m_loader.GetTree(); }
    DgnDbR GetDgnDb() const { return GetTree().GetDgnDb(); }
    bool Is3d() const { return GetTree().Is3d(); }
    bool Is2d() const { return GetTree().Is2d(); }
    LoaderCR GetLoader() const { return m_loader; }
    double GetTolerance() const { return m_tolerance; }
    Render::Primitives::GeometryCollectionCR GetGeometry() const { return m_geometry; }
    ContentIdCR GetContentId() const { return m_loader.GetContentId(); }
    double GetSizeMultiplier() const { return GetContentId().GetSizeMultiplier(); }
    Render::SystemR GetRenderSystem() const { return GetTree().GetRenderSystem(); }
    DRange3dCR GetContentRange() const { return m_contentRange; }
    bool IsCanceled() const { return m_loader.IsCanceled(); }

    DRange3dCR GetTileRange() const { return m_range; }
    DRange3d GetDgnRange() const
        {
        DRange3d range;
        GetTree().GetLocation().Multiply(range, GetTileRange());
        return range;
        }

    bool GenerateGeometry();
};

//=======================================================================================
//! Populates a set of the largest N elements within a range, excluding any elements below
//! a specified size. Keeps track of whether any elements were skipped.
// @bsistruct                                                   Paul.Connelly   05/17
//=======================================================================================
struct ElementCollector : RangeIndex::Traverser
{
    typedef bmultimap<double, DgnElementId, std::greater<double>> Entries;
private:
    Entries         m_entries;
    FBox3d          m_range;
    double          m_minRangeDiagonalSquared;
    uint32_t        m_maxElements;
    LoaderCR        m_loader;
    size_t          m_numSkipped = 0;
    bool            m_aborted = false;
    bool            m_is2d;

    bool CheckStop() { return m_aborted || (m_aborted = m_loader.IsCanceled()); }

    void Insert(double diagonalSq, DgnElementId elemId)
        {
        m_entries.Insert(diagonalSq, elemId);
        if (m_entries.size() > m_maxElements)
            {
            m_entries.erase(--m_entries.end()); // remove the smallest element.
            ++m_numSkipped;
            }
        }

    Accept _CheckRangeTreeNode(FBox3d const& box, bool is3d) const override
        {
        if (!m_aborted && box.IntersectsWith(m_range))
            return Accept::Yes;
        else
            return Accept::No;
        }

    Stop _VisitRangeTreeEntry(RangeIndex::EntryCR entry) override
        {
        if (CheckStop())
            return Stop::Yes;
        else if (!entry.m_range.IntersectsWith(m_range))
            return Stop::No; // why do we need to check the range again here? _CheckRangeTreeNode() should have handled it, but doesn't...

        double sizeSq;
        if (Placement3d::IsMinimumRange(entry.m_range.m_low, entry.m_range.m_high, m_is2d))
            sizeSq = 0.0;
        else if (m_is2d)
            sizeSq = entry.m_range.m_low.DistanceSquaredXY(entry.m_range.m_high);
        else
            sizeSq = entry.m_range.m_low.DistanceSquared(entry.m_range.m_high);

        if (0.0 == sizeSq || sizeSq >= m_minRangeDiagonalSquared)
            Insert(sizeSq, entry.m_id);
        else
            ++m_numSkipped;
        
        return Stop::No;
        }
public:
    ElementCollector(DRange3dCR range, RangeIndex::Tree& rangeIndex, double minRangeDiagonalSquared, LoaderCR loader, uint32_t maxElements)
        : m_range(range), m_minRangeDiagonalSquared(minRangeDiagonalSquared), m_maxElements(maxElements), m_loader(loader), m_is2d(!rangeIndex.Is3d())
        {
        // ###TODO: Do not traverse if tile is not expired (only deletions may have occurred)...
        rangeIndex.Traverse(*this);
        }

    bool AnySkipped() const { return 0 < m_numSkipped; }
    size_t GetNumSkipped() const { return m_numSkipped; }
    Entries const& GetEntries() const { return m_entries; }
    uint32_t GetMaxElements() const { return m_maxElements; }
};

/*---------------------------------------------------------------------------------**//**
* This exists because DRange3d::IntersectionOf() treats a zero-thickness intersection as
* null - so if the intersection in any dimension is zero, it nulls out the entire range.
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void clipContentRangeToTileRange(DRange3dR content, DRange3dCR tile)
    {
    content.low.x = std::max(content.low.x, tile.low.x);
    content.low.y = std::max(content.low.y, tile.low.y);
    content.low.z = std::max(content.low.z, tile.low.z);
    content.high.x = std::min(content.high.x, tile.high.x);
    content.high.y = std::min(content.high.y, tile.high.y);
    content.high.z = std::min(content.high.z, tile.high.z);

    if (content.low.x > content.high.x || content.low.y > content.high.y || content.low.z > content.high.z)
        content.Init();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static DRange3d bisectRange2d(DRange3dCR range, bool takeLow)
    {
    DVec3d diag = range.DiagonalVector();
    DRange3d subRange = range;

    double bisect;
    double* replace;
    if (diag.x > diag.y)
        {
        bisect = (range.low.x + range.high.x) / 2.0;
        replace = takeLow ? &subRange.high.x : &subRange.low.x;
        }
    else
        {
        bisect = (range.low.y + range.high.y) / 2.0;
        replace = takeLow ? &subRange.high.y : &subRange.low.y;
        }

    *replace = bisect;
    return subRange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
static DRange3d bisectRange(DRange3dCR range, bool takeLow)
    {
    DVec3d diag = range.DiagonalVector();
    DRange3d subRange = range;

    double bisect;
    double* replace;
    if (diag.x > diag.y && diag.x > diag.z)
        {
        bisect = (range.low.x + range.high.x) / 2.0;
        replace = takeLow ? &subRange.high.x : &subRange.low.x;
        }
    else if (diag.y > diag.z)
        {
        bisect = (range.low.y + range.high.y) / 2.0;
        replace = takeLow ? &subRange.high.y : &subRange.low.y;
        }
    else
        {
        bisect = (range.low.z + range.high.z) / 2.0;
        replace = takeLow ? &subRange.high.z : &subRange.low.z;
        }

    *replace = bisect;
    return subRange;
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
struct RangeAccumulator : RangeIndex::Traverser
{
    DRange3dR       m_range;
    uint32_t        m_numElements = 0;
    bool            m_is2d;

    RangeAccumulator(DRange3dR range, bool is2d) : m_range(range), m_is2d(is2d) { m_range = DRange3d::NullRange(); }

    bool _AbortOnWriteRequest() const final { return true; }
    Accept _CheckRangeTreeNode(FBox3d const&, bool) const final { return Accept::Yes; }
    Stop _VisitRangeTreeEntry(RangeIndex::EntryCR entry) final
        {
        ++m_numElements;
        m_range.Extend(entry.m_range.ToRange3d());
        return Stop::No;
        }

    bool Accumulate(RangeIndex::Tree& tree)
        {
        if (Stop::Yes == tree.Traverse(*this))
            return false;
        else
            return !m_range.IsNull();
        }

    uint32_t GetElementCount() const { return m_numElements; }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
static DRange3d scaleSpatialRange(DRange3dCR range)
    {
    // Geometry often lies in a plane precisely coincident with the planes of the project extents.
    // We must expand the extents slightly to prevent floating point fuzz in range intersection tests
    // against such geometry - otherwise portions may be inappropriately culled.
    // Similarly (TFS#863543) some 3d data sets consist of essentially 2d data sitting smack in the center of the
    // project extents. If we subdivide tiles in half, we face similar issues where the geometry ends up precisely
    // aligned to tile boundaries. Bias the scale to prevent this.
    // NOTE: There's no simple way to detect and deal with arbitrarily located geometry just-so-happening to
    // align with some tile's boundary...
    constexpr double loScale = 1.0001,
                     hiScale = 1.0002,
                     fLo = 0.5 * (1.0 + loScale),
                     fHi = 0.5 * (1.0 + hiScale);

    DRange3d result = range;
    if (!result.IsNull())
        {
        result.high.Interpolate(range.low, fHi, range.high);
        result.low.Interpolate(range.high, fLo, range.low);
        }

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool isElementCountLessThan(uint32_t threshold, RangeIndex::Tree& tree)
    {
    struct Counter : RangeIndex::Traverser
    {
        uint32_t    m_count = 0;
        uint32_t    m_threshold;

        explicit Counter(uint32_t threshold) : m_threshold(threshold) { }

        Accept _CheckRangeTreeNode(FBox3d const&, bool) const final { return m_count < m_threshold ? Accept::Yes : Accept::No; }
        Stop _VisitRangeTreeEntry(RangeIndex::EntryCR entry) final
            {
            ++m_count;
            return m_count < m_threshold ? Stop::No : Stop::Yes;
            }
    };

    Counter counter(threshold);
    tree.Traverse(counter);
    return counter.m_count < threshold;
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   05/17
//=======================================================================================
struct TileBuilder : GeometryListBuilder
{
protected:
    TileContext&                m_context;
    double                      m_rangeDiagonalSquared;

    static void AddNormals(PolyfaceHeaderR, IFacetOptionsR);
    static void AddParams(PolyfaceHeaderR, IFacetOptionsR);

    void _AddPolyface(PolyfaceQueryCR, bool) override;
    void _AddPolyfaceR(PolyfaceHeaderR, bool) override;
    void _AddSubGraphic(GraphicR, TransformCR, GraphicParamsCR, ClipVectorCP) override;
    bool _WantStrokeLineStyle(LineStyleSymbCR, IFacetOptionsPtr&) override;
    bool _WantPreBakedBody(IBRepEntityCR) override;

    GraphicBuilderPtr _CreateSubGraphic(TransformCR, ClipVectorCP) const override;
    GraphicPtr _FinishGraphic(GeometryAccumulatorR) override;
    void _ActivateGraphicParams(GraphicParamsCR gfParams, GeometryParamsCP geomParams) override
        {
        BeAssert(nullptr == geomParams || geomParams->GetSubCategoryId().IsValid());
        GeometryListBuilder::_ActivateGraphicParams(gfParams, geomParams);
        }

    TileBuilder(TileContext& context, DRange3dCR range);

    void ReInitialize(DRange3dCR range);
public:
    TileBuilder(TileContext& context, DgnElementId elemId, double rangeDiagonalSquared, CreateParams const& params);

    void ReInitialize(DgnElementId elemId, double rangeDiagonalSquared, TransformCR localToWorld);
    double GetRangeDiagonalSquared() const { return m_rangeDiagonalSquared; }
};

DEFINE_POINTER_SUFFIX_TYPEDEFS(TileBuilder);
DEFINE_REF_COUNTED_PTR(TileBuilder);

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   05/17
//=======================================================================================
struct TileSubGraphic : TileBuilder
{
private:
    DgnGeometryPartCPtr m_input;
    GeomPartPtr         m_output;

    GraphicPtr _FinishGraphic(GeometryAccumulatorR) override;
    void _Reset() override { m_input = nullptr; m_output = nullptr; }
public:
    TileSubGraphic(TileContext& context, DgnGeometryPartCP part = nullptr);
    TileSubGraphic(TileContext& context, DgnGeometryPartCR part) : TileSubGraphic(context, &part) { }

    void ReInitialize(DgnGeometryPartCR part);

    DgnGeometryPartCR GetInput() const { BeAssert(m_input.IsValid()); return *m_input; }
    GeomPartPtr GetOutput() const { return m_output; }
    void SetOutput(GeomPartR output) { BeAssert(m_output.IsNull()); m_output = &output; }
};

DEFINE_POINTER_SUFFIX_TYPEDEFS(TileSubGraphic);
DEFINE_REF_COUNTED_PTR(TileSubGraphic);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
static bool wantGlyphBoxes(double sizeInPixels)
    {
    constexpr double s_glyphBoxSizeThreshold = 3.0;
    return sizeInPixels <= s_glyphBoxSizeThreshold;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
static bool setupForStyledCurveVector(GraphicBuilderR builder, CurveVectorCR curve)
    {
    BeAssert(nullptr != dynamic_cast<TileBuilderP>(&builder));
    auto& tileBuilder = static_cast<TileBuilderR>(builder);
    bool wasCurved = tileBuilder.IsAddingCurved();
    tileBuilder.SetAddingCurved(curve.ContainsNonLinearPrimitive());
    return wasCurved;
    }

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     12/2017
+===============+===============+===============+===============+===============+======*/
struct TileRangeClipOutput : PolyfaceQuery::IClipToPlaneSetOutput
{
    bvector<PolyfaceHeaderPtr>  m_clipped;
    bvector<PolyfaceQueryCP>    m_output;

    StatusInt _ProcessUnclippedPolyface(PolyfaceQueryCR mesh) override { m_output.push_back(&mesh); ; return SUCCESS; }
    StatusInt _ProcessClippedPolyface(PolyfaceHeaderR mesh) override { m_output.push_back(&mesh); m_clipped.push_back(&mesh); return SUCCESS; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   02/17
//=======================================================================================
struct MeshGenerator : ViewContext
{
private:
    GeometryLoaderCR        m_loader;
    GeometryOptions         m_options;
    size_t                  m_geometryCount = 0;
    FeatureTable            m_featureTable;
    MeshBuilderMap          m_builderMap;
    DRange3d                m_contentRange = DRange3d::NullRange();
    ThematicMeshBuilderPtr  m_thematicMeshBuilder;
    bool                    m_maxGeometryCountExceeded = false;
    bool                    m_didDecimate = false;

    static constexpr size_t GetDecimatePolyfacePointCount() { return 100; }

    MeshBuilderR GetMeshBuilder(MeshBuilderMap::Key const& key);
    DgnElementId GetElementId(GeometryR geom) const { return m_maxGeometryCountExceeded ? DgnElementId() : geom.GetEntityId(); }

    void AddPolyfaces(GeometryR geom, double rangePixels, bool isContained);
    void AddPolyfaces(PolyfaceList& polyfaces, GeometryR geom, double rangePixels, bool isContained);
    void AddPolyface(Polyface& polyface, GeometryR, double rangePixels, bool isContained);
    void AddClippedPolyface(PolyfaceQueryCR, DgnElementId, DisplayParamsCR, MeshEdgeCreationOptions, bool isPlanar);

    void AddStrokes(GeometryR geom, double rangePixels, bool isContained);
    void AddStrokes(StrokesList& strokes, GeometryR geom, double rangePixels, bool isContained);
    void AddStrokes(StrokesR strokes, GeometryR geom, double rangePixels, bool isContained);
    Strokes ClipSegments(StrokesCR strokes) const;
    void ClipStrokes(StrokesR strokes) const;
    void ClipPoints(StrokesR strokes) const;

    SystemP _GetRenderSystem() const final { return &m_loader.GetRenderSystem(); }
    GraphicBuilderPtr _CreateGraphic(GraphicBuilder::CreateParams const&) final { BeAssert(false); return nullptr; }
    GraphicPtr _CreateBranch(GraphicBranch&, DgnDbR, TransformCR, ClipVectorCP) final { BeAssert(false); return nullptr; }
    double _GetPixelSizeAtPoint(DPoint3dCP) const final { return GetTolerance(); }
    bool _WantGlyphBoxes(double sizeInPixels) const final { return wantGlyphBoxes(sizeInPixels); }
    void _DrawStyledCurveVector(GraphicBuilderR builder, CurveVectorCR curve, GeometryParamsR params, bool doCook) final
        {
        bool wasCurved = setupForStyledCurveVector(builder, curve);
        ViewContext::_DrawStyledCurveVector(builder, curve, params, doCook);
        static_cast<TileBuilderR>(builder).SetAddingCurved(wasCurved);
        }
    bool _AnyPointVisible(DPoint3dCP worldPoints, int nPts, double tolerance) final
        {
        DRange3d        pointRange = DRange3d::From(worldPoints, nPts);
        return pointRange.IntersectsWith(m_loader.GetTileRange());
        }
public:
    MeshGenerator(GeometryLoaderCR loader, GeometryOptionsCR options);

    bool DidDecimation() const { return m_didDecimate; }

    // Add meshes to the MeshBuilder map
    void AddMeshes(GeometryList const& geometries, bool doRangeTest);
    void AddMeshes(GeometryR geom, bool doRangeTest);
    void AddMeshes(GeomPartR part, bvector<GeometryCP> const& instances);

    // Return a list of all meshes currently in the builder map
    MeshList GetMeshes();
    // Return a tight bounding volume
    DRange3dCR GetContentRange() const { return m_contentRange; }
    DRange3dCR GetTileRange() const { return m_builderMap.GetRange(); }
    double GetTolerance() const { return m_loader.GetTolerance(); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
static Feature featureFromParams(DgnElementId elemId, DisplayParamsCR params)
    {
    return Feature(elemId, params.GetSubCategoryId(), params.GetClass());
    }

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ContentId::ToString() const
    {
    Utf8String str;
    Utf8Char buf[BeInt64Id::ID_STRINGBUFFER_LENGTH];
    uint64_t parts[] = { static_cast<uint64_t>(m_depth), m_i, m_j, m_k, static_cast<uint64_t>(m_mult) };
    uint32_t nSeparators = 0;
    for (auto part : parts)
        {
        BeStringUtilities::FormatUInt64(buf, BeInt64Id::ID_STRINGBUFFER_LENGTH, part, HexFormatOptions::None);
        str.append(nSeparators, '/');
        nSeparators = 1;
        str.append(buf);
        }

    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentId::FromString(Utf8CP str)
    {
    if (Utf8String::IsNullOrEmpty(str))
        return false;

    uint64_t i, j, k;
    uint8_t depth, mult;
    if (5 != BE_STRING_UTILITIES_UTF8_SSCANF(str, "%" SCNx8 "/%" SCNx64 "/%" SCNx64 "/%" SCNx64 "/%" SCNx8, &depth, &i, &j, &k, &mult))
        return false;

    *this = ContentId(depth, i, j, k, mult);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
NodeId NodeId::ComputeParentId() const
    {
    auto depth = GetDepth();
    if (depth <= 1)
        {
        BeAssert(depth == 1);
        return RootId();
        }

    return NodeId(depth - 1, m_i >> 2, m_j >> 2, m_k >> 2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d NodeId::ComputeRange(DRange3dCR rootRange, bool is2d) const
    {
    if (IsRoot() || rootRange.IsNull())
        return rootRange;

    auto parentId = ComputeParentId();
    auto parentRange = parentId.ComputeRange(rootRange, is2d);

    bool lowI = 0 == m_i % 2,
         lowJ = 0 == m_j % 2,
         lowK = 0 == m_k % 2;

    // We should never subdivide along z for 2d tiles...###TODO Use a quad-tree for 2d, not an oct-tree.
    auto bisect = is2d ? bisectRange2d : bisectRange;
    DRange3d range = bisect(parentRange, lowI);
    range = bisect(range, lowJ);
    range = bisect(range, lowK);

    return range;
    }

/*---------------------------------------------------------------------------------**//**
* Create the table to hold entries in this Cache
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Cache::_Prepare() const 
    {
    // When current tables were TileTreeCreateTime2 and TileTree3, we changed the file extension from .TileCache to .Tiles
    // So we will never encounter an existing .Tiles file containing previous versions of those tables.
    if (m_db.TableExists(TABLE_NAME_TileTree))
        {
        if (!ValidateData())
            {
            // The db schema is current, but the binary data format is not. Discard it.
            CachedStatementPtr stmt;
            m_db.GetCachedStatement(stmt, "DELETE FROM " TABLE_NAME_TileTreeCreateTime);
            if (BE_SQLITE_DONE != stmt->Step())
                {
                BeAssert(false && "Failed to delete contents of TileTreeCreateTime table");
                }

            m_db.GetCachedStatement(stmt, "DELETE FROM " TABLE_NAME_TileTree);
            if (BE_SQLITE_DONE != stmt->Step())
                {
                BeAssert(false && "Failed to delete contents of TileTree table");
                }
            }

        return SUCCESS;
        }
        
    // Drop leftover tables from previous versions
    m_db.DropTableIfExists(TABLE_NAME_TileTree3);

    // Create the tables
    if (!m_db.TableExists(TABLE_NAME_TileTreeCreateTime) && BE_SQLITE_OK != m_db.CreateTable(TABLE_NAME_TileTreeCreateTime, "TileId CHAR PRIMARY KEY,Created BIGINT"))
        return ERROR;

    return BE_SQLITE_OK == m_db.CreateTable(TABLE_NAME_TileTree,
        "TileId CHAR PRIMARY KEY,Data BLOB,DataSize BIGINT,Metadata TEXT") ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Cache::_Cleanup() const 
    {
    AccessLock lock(const_cast<Cache&>(*this));

    CachedStatementPtr sumStatement;
    m_db.GetCachedStatement(sumStatement, "SELECT SUM(DataSize) FROM " TABLE_NAME_TileTree);

    if (BE_SQLITE_ROW != sumStatement->Step())
        {
        BeAssert(false);
        return ERROR;
        }

    uint64_t sum = sumStatement->GetValueInt64(0);
    if (sum <= m_allowedSize)
        return SUCCESS;

    uint64_t garbageSize = sum - static_cast<uint64_t>(m_allowedSize * .95); // 5% slack to avoid purging often

    CachedStatementPtr selectStatement;
    constexpr Utf8CP selectSql = "SELECT DataSize,Created FROM " JOIN_TileTreeTables " ORDER BY Created ASC";

    m_db.GetCachedStatement(selectStatement, selectSql);
    BeAssert(selectStatement.IsValid());

    uint64_t runningSum=0;
    while (runningSum < garbageSize)
        {
        if (BE_SQLITE_ROW != selectStatement->Step())
            {
            BeAssert(false);
            return ERROR;
            }

        runningSum += selectStatement->GetValueInt64(0);
        }

    BeAssert(runningSum >= garbageSize);
    uint64_t creationDate = selectStatement->GetValueInt64(1);
    BeAssert(creationDate > 0);

    // ###TODO: We should be using foreign key + cascading delete here...
    CachedStatementPtr deleteDataStatement;
    constexpr Utf8CP deleteDataSql = "DELETE FROM " TABLE_NAME_TileTree " WHERE TileId IN"
        " (SELECT TileId FROM " TABLE_NAME_TileTreeCreateTime " WHERE Created <= ?)";
    m_db.GetCachedStatement(deleteDataStatement, deleteDataSql);
    BeAssert(deleteDataStatement.IsValid());
    deleteDataStatement->BindInt64(1, creationDate);
    if (BE_SQLITE_DONE != deleteDataStatement->Step())
        {
        BeAssert(false);
        return ERROR;
        }

    CachedStatementPtr deleteCreatedStatement;
    constexpr Utf8CP deleteCreatedSql = "DELETE FROM " TABLE_NAME_TileTreeCreateTime " WHERE Created <= ?";
    m_db.GetCachedStatement(deleteCreatedStatement, deleteCreatedSql);
    BeAssert(deleteCreatedStatement.IsValid());
    deleteCreatedStatement->BindInt64(1, creationDate);
    if (BE_SQLITE_DONE != deleteCreatedStatement->Step())
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RealityData::CachePtr Cache::Create(DgnDbCR db)
    {
    RealityData::CachePtr cache(new Cache(1024*1024*1024));
    BeFileName cacheName = db.GetFileName();
    cacheName.AppendExtension(L"Tiles");
    if (SUCCESS != cache->OpenAndPrepare(cacheName))
        cache = nullptr;

    return cache;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP Cache::GetCurrentVersion()
    {
    // Increment this when the binary tile format changes...
    // We changed the cache db schema shortly after creating imodel02 branch - so version history restarts at same time.
    // 0: Initial version following db schema change
    // 1: Do not set 'is leaf' if have size multiplier ('zoom factor'); add flag and value for size multiplier
    return "0";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Cache::WriteCurrentVersion() const
    {
    // NB: SavePropertyString() is non-const because modifying *cacheable* properties mutates the Db's internal state
    // Our property is *not* cacheable
    auto& db = const_cast<BeSQLite::Db&>(m_db);
    if (BeSQLite::BE_SQLITE_OK != db.SavePropertyString(GetVersionSpec(), GetCurrentVersion()))
        {
        BeAssert(false && "Failed to save tile cache version");
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Cache::_Initialize() const
    {
    // We've created a brand-new cache Db. Write the current binary format version to its property table.
    return WriteCurrentVersion() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Cache::ValidateData() const
    {
    auto spec = GetVersionSpec();
    Utf8String storedVersion;
    if (BE_SQLITE_ROW == m_db.QueryProperty(storedVersion, spec) && storedVersion.Equals(GetCurrentVersion()))
        return true;

    // Binary format has changed. Discard existing tile data.
    WriteCurrentVersion();
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
TreePtr Tree::Create(GeometricModelR model, Render::SystemR system, TreeType type)
    {
    if (DgnDbStatus::Success != model.FillRangeIndex())
        return nullptr;

    // ###TODO: determine RefinementMode for root tile, store in Tree.
    // bool populateRootTile;
    DRange3d range;
    if (model.Is3dModel())
        {
        range = model.GetDgnDb().GeoLocation().GetProjectExtents();
        range = scaleSpatialRange(range);
        // populateRootTile = isElementCountLessThan(s_minElementsPerTile, *model.GetRangeIndex());
        }
    else
        {
        RangeAccumulator accum(range, model.Is2dModel());
        if (!accum.Accumulate(*model.GetRangeIndex()))
            range = DRange3d::From(DPoint3d::FromZero());

        auto sheet = model.ToSheetModel();
        if (nullptr != sheet)
            range.Extend(sheet->GetSheetExtents());

        // populateRootTile = accum.GetElementCount() < s_minElementsPerTile;
        }
    
    // Translate world coordinates to center of range in order to reduce precision errors
    DPoint3d centroid = DPoint3d::FromInterpolate(range.low, 0.5, range.high);
    Transform transform = Transform::From(centroid);

#if defined (BENTLEYCONFIG_PARASOLID)
    PSolidKernelManager::StartSession();

    if (s_psolidMainThreadMark.IsNull())
        s_psolidMainThreadMark = new PSolidThreadUtil::MainThreadMark();
#endif

    Transform rangeTransform;
    rangeTransform.InverseOf(transform);
    DRange3d tileRange;
    rangeTransform.Multiply(tileRange, range);

    return TreeType::Classifier == type ? new ClassificationTree(model, transform, tileRange, system) : new Tree(model, transform, tileRange, system);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Tree::Tree(GeometricModelCR model, TransformCR location, DRange3dCR range, Render::SystemR system)
    : m_db(model.GetDgnDb()), m_location(location), m_renderSystem(system), m_modelId(model.GetModelId()), m_is3d(model.Is3d()), m_cache(model.GetDgnDb().ElementTileCache())
    {
    // ###TODO: Cache changes to Tile::Cache...
    m_range.Extend(range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Tree::~Tree()
    {
    CancelAllTileLoads();
    WaitForAllLoads();
    BeAssert(m_activeLoads.empty());
    m_cache = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value Tree::ToJson() const
    {
    Json::Value json(Json::objectValue);

    json["id"] = _GetId();
    json["maxTilesToSkip"] = 1;
    json["tileScreenSize"] = s_tileScreenSize;
    JsonUtils::TransformToJson(json["location"], GetLocation());

    // ###TODO: root tile refinement mode...

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Tree::CancelAllTileLoads()
    {
    BeMutexHolder lock(m_cv.GetMutex());
    for (auto& load : m_activeLoads)
        load->SetCanceled();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
ContentCPtr Tree::RequestContent(ContentIdCR contentId)
    {
    LoaderPtr loader;
    BeMutexHolder lock(m_cv.GetMutex());
    auto iter = m_activeLoads.find(contentId);
    if (iter == m_activeLoads.end())
        {
        // Load the tile content synchronously on this thread
        loader = CreateLoader(contentId);
        LoaderScope scope(*loader, lock);
        }
    else
        {
        // Content is already being loaded on another thread. Await the result on this thread.
        loader = *iter;
        auto condition = [&](BeConditionVariable&) { return !loader->IsLoading(); };
        ConditionVariablePredicate<decltype(condition)> predicate(condition);
        m_cv.ProtectedWaitOnCondition(lock, &predicate, BeConditionVariable::Infinite);
        }

    return loader->IsReady() ? loader->GetContent() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Tree::LoaderScope::LoaderScope(LoaderR loader, BeMutexHolder& lock) : m_loader(&loader), m_lock(lock)
    {
    // NB: Mutex is held by this thread on entry...
    loader.GetTree().m_activeLoads.insert(m_loader);

    // Release mutex while loading...
    lock.unlock();
    loader.Perform();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Tree::LoaderScope::~LoaderScope()
    {
    // Ensure loader removed and waiting threads notified even if exception occurs during Loader::Perform().
    m_lock.lock();
    auto& loads = m_loader->GetTree().m_activeLoads;
    BeAssert(loads.end() != loads.find(m_loader));
    loads.erase(m_loader);
    BeAssert(loads.end() == loads.find(m_loader));
    m_loader->GetTree().m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Loader::Loader(TreeR tree, ContentIdCR contentId) : m_contentId(contentId), m_tree(tree), m_cacheKey(tree.ConstructCacheKey(contentId)),
    m_createTime(tree.FetchModel()->GetLastElementModifiedTime())
    {
    BeAssert(State::Loading == GetState()); // zero-initialized...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Loader::~Loader()
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Loader::IsExpired(uint64_t createTimeMillis) const
    {
    // ###TODO: For now assuming model has not been modified since loader was created...
    return createTimeMillis < m_createTime;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Loader::IsValidData(ByteStreamCR cacheData) const
    {
    // ###TODO: We used to verify the feature table in order to detect element updates which had been *undone* - the only sort of change
    // not detectable by checking GeometricModel::GetLastElementModifiedTime().
    // For now not concerned about that case - any other modification is handled by IsExpired().
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Loader::DropFromDb(RealityData::CacheR cache)
    {
    CachedStatementPtr stmt;
    cache.GetDb().GetCachedStatement(stmt, "DELETE FROM " TABLE_NAME_TileTree " WHERE TileId=?");
    stmt->BindText(1, m_cacheKey, Statement::MakeCopy::No);
    if (BE_SQLITE_DONE != stmt->Step())
        {
        BeAssert(false);
        return ERROR;
        }

    cache.GetDb().GetCachedStatement(stmt, "DELETE FROM " TABLE_NAME_TileTreeCreateTime " WHERE TileId=?");
    stmt->BindText(1, m_cacheKey, Statement::MakeCopy::No);
    if (BE_SQLITE_DONE != stmt->Step())
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Loader::Perform()
    {
    BeAssert(IsLoading());
    if (IsCanceled())
        return;

    auto state = ReadFromCache();
    if (IsCanceled())
        return;

    if (State::NotFound == state)
        {
        state = ReadFromModel();
        if (IsCanceled())
            return;
        }

    SetState(state);
    BeAssert(!IsLoading());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Loader::State Loader::ReadFromCache()
    {
    BeAssert(IsLoading());
    auto cache = GetTree().GetCache();
    if (nullptr == cache)
        return State::Invalid;

    if (true)
        {
        RealityData::Cache::AccessLock lock(*cache); // block writes to cache while we're reading

        enum Column : int { Data, DataSize, Created, Rowid };
        CachedStatementPtr stmt;
        constexpr Utf8CP selectSql = "SELECT Data,DataSize,Created," TABLE_NAME_TileTree ".ROWID as TileRowId"
            " FROM " JOIN_TileTreeTables " WHERE " COLUMN_TileTree_TileId "=?";

        if (BE_SQLITE_OK != cache->GetDb().GetCachedStatement(stmt, selectSql))
            return State::Invalid;
        
        stmt->ClearBindings();
        stmt->BindText(1, m_cacheKey, Statement::MakeCopy::No);
        if (BE_SQLITE_ROW != stmt->Step())
            return State::NotFound;

        uint64_t rowId = stmt->GetValueInt64(Column::Rowid);
        uint64_t createTime = stmt->GetValueInt64(Column::Created);
        if (IsExpired(createTime))
            {
            DropFromDb(*cache);
            return State::NotFound;
            }

        if (0 == stmt->GetValueInt64(Column::DataSize))
            {
            m_content = new Content(ByteStream());
            }
        else
            {
            // Potentially too large to allocate on stack...
            auto snappy = std::make_unique<BeSQLite::SnappyFromBlob>();
            if (ZIP_SUCCESS != snappy->Init(cache->GetDb(), TABLE_NAME_TileTree, "Data", rowId))
                return State::Invalid;

            CacheBlobHeader header(*snappy);
            uint32_t sizeRead;
            if (CacheBlobHeader::DB_Signature06 != header.m_signature || 0 == header.m_size)
                return State::Invalid;

            ByteStream bytes;
            bytes.Resize(header.m_size);
            snappy->ReadAndFinish(bytes.GetDataP(), header.m_size, sizeRead);
            if (sizeRead != header.m_size)
                return State::Invalid;

            if (!IsValidData(bytes))
                {
                DropFromDb(*cache);
                return State::NotFound;
                }

            m_content = new Content(std::move(bytes));
            }

        // We've loaded data from cache. Update timestamp.
        if (BE_SQLITE_OK == cache->GetDb().GetCachedStatement(stmt, "UPDATE " TABLE_NAME_TileTreeCreateTime " SET Created=? WHERE TileId=?"))
            {
            stmt->BindInt64(1, GetCreateTime());
            stmt->BindText(2, m_cacheKey, Statement::MakeCopy::No);
            if (BE_SQLITE_DONE != stmt->Step())
                BeAssert(false);
            }
        }

    return State::Ready;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Loader::State Loader::ReadFromModel()
    {
    GeometryLoader geomLoader(*this);
    if (!geomLoader.GenerateGeometry())
        return State::Invalid;
    else if (IsCanceled())
        return State::NotFound; // doesn't matter...

    // ###TODO: Save to cache etc
    return State::Ready;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationLoader::ClassificationLoader(ClassificationTree& tree, ContentIdCR contentId) : T_Super(tree, contentId), m_offset(tree.m_classifierOffset), m_range(tree.m_classifierRange)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/18
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr ClassificationLoader::Preprocess(PolyfaceHeaderR pf) const
    {
    DPlane3d plane;
    if (pf.IsClosedPlanarRegion(plane))
        {
        DRay3d              ray = DRay3d::FromOriginAndVector(plane.origin, plane.normal);
        DRange1d            classifiedProjection = m_range.GetCornerRange(ray);
        PolyfaceHeaderPtr   extrudedPolyface = pf.ComputeOffset(PolyfaceHeader::OffsetOptions(), classifiedProjection.high, classifiedProjection.low);

        if (extrudedPolyface.IsValid())
            {
            extrudedPolyface->Triangulate();
            extrudedPolyface->BuildPerFaceNormals();
            return extrudedPolyface;
            }
        }

    return &pf;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryLoader::GeometryLoader(LoaderCR loader) : m_loader(loader)
    {
    m_range = loader.GetContentId().ComputeRange(GetTree().GetRange(), Is2d());
    double diagDist = Is3d() ? m_range.DiagonalDistance() : m_range.DiagonalDistanceXY();
    m_tolerance = diagDist / (s_minToleranceRatio * GetSizeMultiplier());
    BeAssert(0.0 != m_tolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryLoader::GenerateGeometry()
    {
    auto model = GetTree().FetchModel();
    if (model.IsNull() || DgnDbStatus::Success != model->FillRangeIndex())
        return false;

    uint32_t maxFeatures = s_hardMaxFeaturesPerTile; // Note: Element != Feature - could have multiple features per element due to differing subcategories/classes within GeometryStream
    maxFeatures = std::min(maxFeatures, GetRenderSystem()._GetMaxFeaturesPerBatch());

    auto tolerance = GetTolerance();
    double minRangeDiagonalSq = s_minRangeBoxSize * tolerance;
    minRangeDiagonalSq *= minRangeDiagonalSq;

    IFacetOptionsPtr facetOptions = Geometry::CreateFacetOptions(tolerance);
    facetOptions->SetHideSmoothEdgesWhenGeneratingNormals(false); // We'll do this ourselves when generating meshes - This will turn on sheet edges that should be hidden (Pug.dgn).

    Transform transformFromDgn;
    transformFromDgn.InverseOf(GetTree().GetLocation());

    // ###TODO: Do not populate the element collection up front - just traverse range index one element at a time
    // (We chiefly did the up-front collection previously in order to support partial tiles - process biggest elements first)
    DRange3d dgnRange = GetDgnRange();
    ElementCollector elementCollector(dgnRange, *model->GetRangeIndex(), minRangeDiagonalSq, m_loader, maxFeatures);

    if (IsCanceled())
        return false;

    GeometryList geometryList;
    if (elementCollector.AnySkipped())
        geometryList.MarkIncomplete();

    // ###TODO TileContext tileContext(geometryList, GetTree(), dgnRange, *facetOptions, transformFromDgn, tolerance, *this);
    MeshGenerator meshGenerator(*this, GeometryOptions());

    if (IsCanceled())
        return false;

    for (ElementCollector::Entries::const_iterator elementIter = elementCollector.GetEntries().begin(); elementCollector.GetEntries().end() != elementIter; ++elementIter)
        {
        // Collect geometry from this element
        geometryList.clear();
        // ###TODO auto const& entry = *elementIter;
        // ###TODO tileContext.ProcessElement(entry.second, entry.first);
        // ###TODO if (tileContext.GetGeometryCount() >= elementCollector.GetMaxElements())
            // ###TODO {
            // ###TODO tileContext.TruncateGeometryLIst(elementCollector.GetMaxElements());
            // ###TODO break;
        // ###TODO }

        // Convert this element's geometry to meshes
        for (auto const& geom : geometryList)
            meshGenerator.AddMeshes(*geom, true);

        if (IsCanceled())
            return false;
        }

    if (meshGenerator.DidDecimation())
        geometryList.MarkIncomplete();

    // ###TODO we previously would determine subdivision strategy here.
    // Instead, record relevant info (e.g., # of elements, presence of curves, any geometry skipped, etc) so frontend can make that determination.

    // Facet all geoemtry
    m_geometry.Meshes() = meshGenerator.GetMeshes();
    m_contentRange.Extend(meshGenerator.GetContentRange());
    if (!geometryList.IsComplete())
        m_geometry.MarkIncomplete();

    if (geometryList.ContainsCurves())
        m_geometry.MarkCurved();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
MeshGenerator::MeshGenerator(GeometryLoaderCR loader, GeometryOptionsCR options)
  : m_loader(loader), m_options(options),
    m_featureTable(loader.GetTree().GetModelId(), loader.GetRenderSystem()._GetMaxFeaturesPerBatch()),
    m_builderMap(loader.GetTolerance(), &m_featureTable, loader.GetTileRange(), loader.Is2d())
    {
    SetDgnDb(loader.GetDgnDb());
    m_is3dView = loader.Is3d();
    // ###TODO SetViewFlags(TileContext::GetDefaultViewFlags());

    // For now always create -- (use first aux channel) - ###TODO: control from UX.
    m_thematicMeshBuilder  = new ThematicMeshBuilder("", "");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
MeshBuilderR MeshGenerator::GetMeshBuilder(MeshBuilderMap::Key const& key)
    {
    return m_builderMap[key];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddMeshes(GeometryList const& geometries, bool doRangeTest)
    {
    for (auto& geom : geometries)
        {
        if (m_loader.IsCanceled())
            break;

        AddMeshes(*geom, doRangeTest);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddMeshes(GeometryR geom, bool doRangeTest)
    {
    DRange3dCR geomRange = geom.GetTileRange();
    double rangePixels = (m_loader.Is3d() ? geomRange.DiagonalDistance() : geomRange.DiagonalDistanceXY()) / GetTolerance();
    if (rangePixels < s_minRangeBoxSize && 0.0 < geomRange.DiagonalDistance()) // ###TODO_ELEMENT_TILE: single point primitives have an empty range...
        return;   // ###TODO: -- Produce an artifact from optimized bounding box to approximate from range.

    bool isContained = !doRangeTest || geomRange.IsContained(GetTileRange());
    if (!m_maxGeometryCountExceeded)
        m_maxGeometryCountExceeded = (++m_geometryCount > m_featureTable.GetMaxFeatures());

    AddPolyfaces(geom, rangePixels, isContained);
    AddStrokes(geom, rangePixels, isContained);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddMeshes(GeomPartR part, bvector<GeometryCP> const& instances)
    {
    auto iter = instances.begin();
    if (instances.end() == iter)
        return;

    // All instances will have the same facet options and range size...
    GeometryCP first = *iter;
    DRange3dCR geomRange = first->GetTileRange();
    double tolerance = GetTolerance();
    double rangePixels = (m_loader.Is3d() ? geomRange.DiagonalDistance() : geomRange.DiagonalDistanceXY()) / tolerance;
    if (rangePixels < s_minRangeBoxSize)
        return;

    // Get the polyfaces and strokes with no transform applied
    PolyfaceList polyfaces = part.GetPolyfaces(tolerance, m_options.m_normalMode, nullptr, *this);
    StrokesList strokes = part.GetStrokes(tolerance, nullptr, *this);

    // For each instance, transform the polyfaces and add them to the mesh
    Transform invTransform = Transform::FromIdentity();
    for (GeometryCP instance : instances)
        {
        bool isContained = instance->GetTileRange().IsContained(GetTileRange());
        Transform instanceTransform = Transform::FromProduct(instance->GetTransform(), invTransform);
        invTransform.InverseOf(instance->GetTransform());
        for (auto& polyface : polyfaces)
            {
            polyface.Transform(instanceTransform);
            AddPolyface(polyface, const_cast<GeometryR>(*instance), rangePixels, isContained);
            }

        for (auto& strokeList : strokes)
            {
            strokeList.Transform(instanceTransform);
            AddStrokes(strokeList, *const_cast<GeometryP>(instance), rangePixels, isContained);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddPolyfaces(GeometryR geom, double rangePixels, bool isContained)
    {
    auto polyfaces = geom.GetPolyfaces(GetTolerance(), m_options.m_normalMode, *this);
    AddPolyfaces(polyfaces, geom, rangePixels, isContained);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddPolyfaces(PolyfaceList& polyfaces, GeometryR geom, double rangePixels, bool isContained)
    {
    for (auto& polyface : polyfaces)
        AddPolyface(polyface, geom, rangePixels, isContained);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddPolyface(Polyface& tilePolyface, GeometryR geom, double rangePixels, bool isContained)
    {
    // TFS#817210
    PolyfaceHeaderPtr polyface = tilePolyface.m_polyface.get();
    if (polyface.IsNull() || 0 == polyface->GetPointIndexCount())
        return;

    polyface = m_loader.GetLoader().Preprocess(*polyface);

    // ###TODO: we don't know if tile is to be treated as a leaf or not...
    bool doDecimate = false; // !m_tile.IsLeaf() && !m_tile.HasZoomFactor() && geom.DoDecimate() && polyface->GetPointCount() > GetDecimatePolyfacePointCount() && 0 == polyface->GetFaceCount();

    if (doDecimate)
        {
        BeAssert(0 == polyface->GetEdgeChainCount());       // The decimation does not handle edge chains - but this only occurs for polyfaces which should never have them.
        PolyfaceHeaderPtr   decimated;
        if (doDecimate && (decimated = polyface->ClusteredVertexDecimate(GetTolerance(), .25 /* No decimation unless point count reduced by at least 25% */)).IsValid())
            {
            polyface = decimated.get();
            m_didDecimate = true;
            }
        }

    auto edgeOptions = tilePolyface.m_displayEdges ? MeshEdgeCreationOptions::DefaultEdges : MeshEdgeCreationOptions::NoEdges;

    DgnElementId            elemId = GetElementId(geom);
    MeshEdgeCreationOptions edges(edgeOptions);
    bool                    isPlanar = tilePolyface.m_isPlanar;
    DisplayParamsCPtr       displayParams = &tilePolyface.GetDisplayParams(); 

    if (m_thematicMeshBuilder.IsValid())
        m_thematicMeshBuilder->InitThematicDisplay(*polyface, *displayParams);

    if (isContained)
        {                                                                                                                                          
        AddClippedPolyface(*polyface, elemId, *displayParams, edges, isPlanar);
        }
    else
        {
        TileRangeClipOutput clipOutput;
        polyface->ClipToRange(GetTileRange(), clipOutput, false);
        for (auto& clipped : clipOutput.m_output)
            AddClippedPolyface(*clipped, elemId, *displayParams, edges, isPlanar);                                                                                        
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddClippedPolyface(PolyfaceQueryCR polyface, DgnElementId elemId, DisplayParamsCR displayParams, MeshEdgeCreationOptions edgeOptions, bool isPlanar)
    {
    bool                hasTexture = displayParams.IsTextured();
    bool                anyContributed = false;
    uint32_t            fillColor = displayParams.GetFillColor();
    DgnDbR              db = m_loader.GetDgnDb();
    MeshAuxData         auxData;
    uint64_t            keyElementId = m_loader.GetLoader().SeparatePrimitivesById() ? elemId.GetValue() : 0; // Create separate primitives per element if classifying.
    MeshBuilderMap::Key key(displayParams, nullptr != polyface.GetNormalIndexCP(), Mesh::PrimitiveType::Mesh, isPlanar, keyElementId);
    MeshBuilderR        builder = GetMeshBuilder(key);

    builder.BeginPolyface(polyface, edgeOptions);

    if (m_thematicMeshBuilder.IsValid())
        m_thematicMeshBuilder->BuildMeshAuxData(auxData, polyface, displayParams);

    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(polyface); visitor->AdvanceToNextFace(); /**/)
        {
        anyContributed = true;
        builder.AddFromPolyfaceVisitor(*visitor, displayParams.GetTextureMapping(), db, featureFromParams(elemId, displayParams), hasTexture, fillColor, nullptr != polyface.GetNormalCP(), &auxData);
        m_contentRange.Extend(visitor->Point());
        }

    builder.EndPolyface();

    if (anyContributed)
        {
        // NB: The mesh's display params contain a fill color, which is used by the tri mesh primitive if the color table is empty (uniform)
        // But each polyface's display params may have a different fill color.
        // If a polyface contributes no vertices, we may end up incorrectly using its fill color for the primitive
        // Make sure the mesh's display params match one (any) mesh which actually contributed vertices, so that if the result is a uniform color,
        // we will use the fill color of the (only) mesh which contributed.
        builder.SetDisplayParams(displayParams);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::ClipStrokes(StrokesR strokes) const
    {
    if (strokes.m_disjoint)
        ClipPoints(strokes);
    else
        strokes = ClipSegments(strokes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/17
+---------------+---------------+---------------+---------------+---------------+------*/
Strokes MeshGenerator::ClipSegments(StrokesCR input) const
    {
    BeAssert(!input.m_disjoint);

    Strokes output(*input.m_displayParams, false, input.m_isPlanar);
    output.m_strokes.reserve(input.m_strokes.size());

    for (auto const& inputStroke : input.m_strokes)
        {
        auto const& points = inputStroke.m_points;
        if (points.size() <= 1)
            continue;

        DPoint3d prevPt = points.front();
        bool prevOutside = !GetTileRange().IsContained(prevPt);
        if (!prevOutside)
            {
            output.m_strokes.push_back(Strokes::PointList(inputStroke.m_startDistance));
            output.m_strokes.back().m_points.push_back(prevPt);
            }

        double length = inputStroke.m_startDistance;        // Cumulative length along polyline
        for (size_t i = 1; i < points.size(); i++)
            {
            auto nextPt = points[i];
            bool nextOutside = !GetTileRange().IsContained(nextPt);
            DSegment3d clippedSegment;
            if (prevOutside || nextOutside)
                {
                double param0, param1;
                DSegment3d unclippedSegment = DSegment3d::From(prevPt, nextPt);
                if (!GetTileRange().IntersectBounded(param0, param1, clippedSegment, unclippedSegment))
                    {
                    // entire segment clipped
                    prevPt = nextPt;
                    continue;
                    }
                }

            DPoint3d startPt = prevOutside ? clippedSegment.point[0] : prevPt;
            DPoint3d endPt = nextOutside ? clippedSegment.point[1] : nextPt;

            if (prevOutside)
                {
                output.m_strokes.push_back(Strokes::PointList(length));
                output.m_strokes.back().m_points.push_back(startPt);
                }

            output.m_strokes.back().m_points.push_back(endPt);

            prevPt = nextPt;
            prevOutside = nextOutside;
            }

        BeAssert(output.m_strokes.empty() || 1 < output.m_strokes.back().m_points.size());
        }

    return output;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::ClipPoints(StrokesR strokes) const
    {
    BeAssert(strokes.m_disjoint);

    for (auto& stroke : strokes.m_strokes)
        {
        auto eraseAt = std::remove_if(stroke.m_points.begin(), stroke.m_points.end(), [&](DPoint3dCR pt) { return !GetTileRange().IsContained(pt); });
        if (stroke.m_points.end() != eraseAt)
            stroke.m_points.erase(eraseAt);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddStrokes(GeometryR geom, double rangePixels, bool isContained)
    {
    auto strokes = geom.GetStrokes(GetTolerance(), *this);
    AddStrokes(strokes, geom, rangePixels, isContained);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddStrokes(StrokesList& strokes, GeometryR geom, double rangePixels, bool isContained)
    {
    for (auto& stroke : strokes)
        AddStrokes(stroke, geom, rangePixels, isContained);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddStrokes(StrokesR strokes, GeometryR geom, double rangePixels, bool isContained)
    {
    if (m_loader.IsCanceled())
        return;

    if (!isContained)
        ClipStrokes(strokes);

    if (strokes.m_strokes.empty())
        return; // avoid potentially creating the builder below...

    DisplayParamsCR     displayParams = strokes.GetDisplayParams();
    MeshBuilderMap::Key key(displayParams, false, strokes.m_disjoint ? Mesh::PrimitiveType::Point : Mesh::PrimitiveType::Polyline, strokes.m_isPlanar);
    MeshBuilderR builder = GetMeshBuilder(key);

    uint32_t fillColor = displayParams.GetLineColor();
    DgnElementId elemId = GetElementId(geom);
    for (auto& stroke : strokes.m_strokes)
        {
        if (stroke.m_points.size() > (strokes.m_disjoint ?  0 : 1))
            {
            m_contentRange.Extend(stroke.m_points);
            builder.AddPolyline(stroke.m_points, featureFromParams(elemId, displayParams), fillColor, stroke.m_startDistance);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
MeshList MeshGenerator::GetMeshes()
    {
    MeshList meshes;
    for (auto& builder : m_builderMap)
        {
        MeshP mesh = builder.second->GetMesh();
        if (!mesh->IsEmpty())
            meshes.push_back(mesh);
        }

    // Do not allow vertices outside of this tile's range to expand its content range
    clipContentRangeToTileRange(m_contentRange, GetTileRange());

    meshes.m_features = std::move(m_featureTable);

    return meshes;
    }



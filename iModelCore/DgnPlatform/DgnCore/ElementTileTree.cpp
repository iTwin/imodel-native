/*--------------------------------------------------------------------------------------+                                                                                           
|
|     $Source: DgnCore/ElementTileTree.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/ElementTileTree.h>
#include <DgnPlatform/TileReader.h>
#include <folly/BeFolly.h>
#include <DgnPlatform/RangeIndex.h>
#include <GeomJsonWireFormat/JsonUtils.h>
#include <numeric>
#include <inttypes.h>
#if defined (BENTLEYCONFIG_PARASOLID) 
#include <DgnPlatform/DgnBRep/PSolidUtil.h>
#endif


// Define this if you want to generate a root tile containing geometry.
// By default the root tile is empty unless it would contain relatively few elements, which enables us to:
//  - reduce the number of elements per top-most tile; and
//  - parallelize the generation of 8 top-most tiles
// thereby improving tile generation speed significantly.
// #define POPULATE_ROOT_TILE

// Uncomment to always use tile repair when reading from cache, instead of only when tile contents invalidated.
// (Must have Platform.TileRepair feature gate enabled too!)
// #define TEST_TILE_REPAIR

USING_NAMESPACE_ELEMENT_TILETREE
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES

using FBox3d = RangeIndex::FBox;

BEGIN_UNNAMED_NAMESPACE

struct TileContext;

// Temporary: Disabling edge generation by default until memory consumption issues resolved. Comment out following to re-enable
// #define DISABLE_EDGE_GENERATION

// For debugging tile generation code - disables use of cached tiles.
// #define DISABLE_TILE_CACHE

// We used to cache GeometryLists for elements occupying a significant (25%) fraction of the total model range.
// That can't work for BReps because they are associated with a specific thread's partition.
// In any case it wasn't much of an optimization as we still had to facet/stroke the geometry each time it was encountered.
// #define CACHE_LARGE_GEOMETRY

// Cache facets for geometry parts in Root
// This cache grows in an unbounded manner - and every BRep is typically a part, even if only one reference to it exists
// With this disabled, we will still ensure that when multiple threads want to facet the same part, all but the first will wait for the first to do so
// NOTE: Caching geometry parts is not 100% reliable, because the symbology associated with each instance can be any combination of that defined in the part's GeometryStream
// and that defined in the referencing GeometryStream. 99.9% of the time no symbology is defined in the part however...
// #define CACHE_GEOMETRY_PARTS

// Turn this on to diagnose issues which may be specific to partial tile generation.
// It will prevent us from creating partial tiles - instead tile generation will take as long as needed to produce a complete tile.
// #define DISABLE_PARTIAL_TILES

// Often we find multiple threads trying to facet the same DgnGeometryPart simultaneously, and we want to allow only one thread to do so while the others wait on the result.
// Unfortunately this is not 100% reliable, because the symbology associated with each instance can be any combination of that defined in the part's GeometryStream
// and that defined in the referencing GeometryStream. 99.9% of the time no symbology is defined in the part however...
// Uncomment this to enable that (ideally after having addressed the symbology issue noted above)
// #define SHARE_GEOMETRY_PARTS

// Uncomment to record and output statistics on # of cached tiles, time spent reading them, etc
// #define TILECACHE_DEBUG

// Uncomment to compare geometry read from tile cache data to that produced by LoadGeometryFromModel() and assert if unequal.
// See TFS#772315 in which portions of geometry do not appear in tiles read from cache, but do appear if we disable the cache
// #define DEBUG_TILE_CACHE_GEOMETRY
#if defined(DEBUG_TILE_CACHE_GEOMETRY)
    #if !defined(DISABLE_PARTIAL_TILES)
        #define DISABLE_PARTIAL_TILES
    #endif
#endif

#ifdef TILECACHE_DEBUG
#define TILECACHE_PRINTF THREADLOG.debugv
#else
#define TILECACHE_PRINTF(...)

#endif

constexpr double s_minRangeBoxSize = 2.5;     // Threshold below which we consider geometry/element too small to contribute to tile mesh ###TODO: Revisit...
constexpr double s_tileScreenSize = 512.0;
constexpr double s_minToleranceRatioMultiplier = 2.0;
constexpr double s_minToleranceRatio = s_tileScreenSize * s_minToleranceRatioMultiplier;
constexpr uint32_t s_minElementsPerTile = 100; // ###TODO: The complexity of a single element's geometry can vary wildly...
constexpr double s_solidPrimitivePartCompareTolerance = 1.0E-5;
constexpr uint32_t s_hardMaxFeaturesPerTile = 2048*1024;
constexpr double s_maxLeafTolerance = 1.0; // the maximum tolerance at which we will stop subdividing tiles, regardless of # of elements contained or whether curved geometry exists.

static Root::DebugOptions s_globalDebugOptions = Root::DebugOptions::None;

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
struct RangeAccumulator : RangeIndex::Traverser
{
    DRange3dR       m_range;
    uint32_t        m_numElements = 0;
    bool            m_is2d;

    RangeAccumulator(DRange3dR range, bool is2d) : m_range(range), m_is2d(is2d) { m_range = DRange3d::NullRange(); }

    bool _AbortOnWriteRequest() const override { return true; }
    Accept _CheckRangeTreeNode(FBox3d const&, bool) const override { return Accept::Yes; }
    Stop _VisitRangeTreeEntry(RangeIndex::EntryCR entry) override
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
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool isElementCountLessThan(uint32_t threshold, RangeIndex::Tree& tree)
    {
    struct Counter : RangeIndex::Traverser
    {
        uint32_t    m_count = 0;
        uint32_t    m_threshold;

        explicit Counter(uint32_t threshold) : m_threshold(threshold) { }

        Accept _CheckRangeTreeNode(FBox3d const&, bool) const override { return m_count < m_threshold ? Accept::Yes : Accept::No; }
        Stop _VisitRangeTreeEntry(RangeIndex::EntryCR entry) override
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
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct TileGeometrySource
{
    struct GeomBlob
    {
        void const* m_blob;
        int         m_size;

        GeomBlob(void const* blob, int size) : m_blob(blob), m_size(size) { }
        template<typename T> GeomBlob(T& stmt, int columnIndex)
            {
            m_blob = stmt.GetValueBlob(columnIndex);
            m_size = stmt.GetColumnBytes(columnIndex);
            }
    };
protected:
    DgnCategoryId           m_categoryId;
    GeometryStream          m_geom;
    DgnDbR                  m_db;
    bool                    m_isGeometryValid;

    TileGeometrySource(DgnCategoryId categoryId, DgnDbR db, GeomBlob const& geomBlob) : m_categoryId(categoryId), m_db(db)
        {
        m_isGeometryValid = DgnDbStatus::Success == db.Elements().LoadGeometryStream(m_geom, geomBlob.m_blob, geomBlob.m_size);
        }
public:
    bool IsGeometryValid() const { return m_isGeometryValid; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct TileGeometrySource3d : TileGeometrySource, GeometrySource3d
{
private:
    Placement3d     m_placement;

    TileGeometrySource3d(DgnCategoryId categoryId, DgnDbR db, GeomBlob const& geomBlob, Placement3dCR placement)
        : TileGeometrySource(categoryId, db, geomBlob), m_placement(placement) { }

    DgnDbR _GetSourceDgnDb() const override { return m_db; }
    DgnElementCP _ToElement() const override { return nullptr; }
    GeometrySource3dCP _GetAsGeometrySource3d() const override { return this; }
    DgnCategoryId _GetCategoryId() const override { return m_categoryId; }
    GeometryStreamCR _GetGeometryStream() const override { return m_geom; }
    Placement3dCR _GetPlacement() const override { return m_placement; }

    DgnDbStatus _SetCategoryId(DgnCategoryId categoryId) override { BeAssert(false && "No reason to access this"); return DgnDbStatus::BadRequest; }
    DgnDbStatus _SetPlacement(Placement3dCR) override { BeAssert(false && "No reason to access this"); return DgnDbStatus::BadRequest; }
public:
    static std::unique_ptr<GeometrySource> Create(DgnCategoryId categoryId, DgnDbR db, GeomBlob const& geomBlob, Placement3dCR placement)
        {
        std::unique_ptr<GeometrySource> pSrc(new TileGeometrySource3d(categoryId, db, geomBlob, placement));
        if (!static_cast<TileGeometrySource3d const&>(*pSrc).IsGeometryValid())
            return nullptr;

        return pSrc;
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
struct TileGeometrySource2d : TileGeometrySource, GeometrySource2d
{
private:
    Placement2d     m_placement;

    TileGeometrySource2d(DgnCategoryId categoryId, DgnDbR db, GeomBlob const& geomBlob, Placement2dCR placement)
        : TileGeometrySource(categoryId, db, geomBlob), m_placement(placement) { }

    DgnDbR _GetSourceDgnDb() const override { return m_db; }
    DgnElementCP _ToElement() const override { return nullptr; }
    GeometrySource2dCP _GetAsGeometrySource2d() const override { return this; }
    DgnCategoryId _GetCategoryId() const override { return m_categoryId; }
    GeometryStreamCR _GetGeometryStream() const override { return m_geom; }
    Placement2dCR _GetPlacement() const override { return m_placement; }

    DgnDbStatus _SetCategoryId(DgnCategoryId categoryId) override { BeAssert(false && "No reason to access this"); return DgnDbStatus::BadRequest; }
    DgnDbStatus _SetPlacement(Placement2dCR) override { BeAssert(false && "No reason to access this"); return DgnDbStatus::BadRequest; }
public:
    static std::unique_ptr<GeometrySource> Create(DgnCategoryId categoryId, DgnDbR db, GeomBlob const& geomBlob, Placement2dCR placement)
        {
        std::unique_ptr<GeometrySource> pSrc(new TileGeometrySource2d(categoryId, db, geomBlob, placement));
        if (!static_cast<TileGeometrySource2d const&>(*pSrc).IsGeometryValid())
            return nullptr;

        return pSrc;
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
struct GeometrySelector3d
{
    static bool Is3d() { return true; }

    static Utf8CP GetSql()
        {
        return "SELECT CategoryId,GeometryStream,Yaw,Pitch,Roll,Origin_X,Origin_Y,Origin_Z,BBoxLow_X,BBoxLow_Y,BBoxLow_Z,BBoxHigh_X,BBoxHigh_Y,BBoxHigh_Z FROM "
                BIS_TABLE(BIS_CLASS_GeometricElement3d) " WHERE ElementId=?";
        }

    static std::unique_ptr<GeometrySource> ExtractGeometrySource(BeSQLite::CachedStatement& stmt, DgnDbR db)
        {
        auto categoryId = stmt.GetValueId<DgnCategoryId>(0);
        TileGeometrySource::GeomBlob geomBlob(stmt, 1);

        DPoint3d origin = DPoint3d::From(stmt.GetValueDouble(5), stmt.GetValueDouble(6), stmt.GetValueDouble(7)),
                 boxLo  = DPoint3d::From(stmt.GetValueDouble(8), stmt.GetValueDouble(9), stmt.GetValueDouble(10)),
                 boxHi  = DPoint3d::From(stmt.GetValueDouble(11), stmt.GetValueDouble(12), stmt.GetValueDouble(13));

        Placement3d placement(origin,
                YawPitchRollAngles(Angle::FromDegrees(stmt.GetValueDouble(2)), Angle::FromDegrees(stmt.GetValueDouble(3)), Angle::FromDegrees(stmt.GetValueDouble(4))),
                ElementAlignedBox3d(boxLo.x, boxLo.y, boxLo.z, boxHi.x, boxHi.y, boxHi.z));

        return TileGeometrySource3d::Create(categoryId, db, geomBlob, placement);
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
struct GeometrySelector2d
{
    static bool Is3d() { return false; }

    static Utf8CP GetSql()
        {
        return "SELECT CategoryId,GeometryStream,Rotation,Origin_X,Origin_Y,BBoxLow_X,BBoxLow_Y,BBoxHigh_X,BBoxHigh_Y FROM "
                BIS_TABLE(BIS_CLASS_GeometricElement2d) " WHERE ElementId=?";
        }

    static std::unique_ptr<GeometrySource> ExtractGeometrySource(BeSQLite::CachedStatement& stmt, DgnDbR db)
        {
        auto categoryId = stmt.GetValueId<DgnCategoryId>(0);
        TileGeometrySource::GeomBlob geomBlob(stmt, 1);

        auto rotation = AngleInDegrees::FromDegrees(stmt.GetValueDouble(2));
        DPoint2d origin = DPoint2d::From(stmt.GetValueDouble(3), stmt.GetValueDouble(4));
        ElementAlignedBox2d bbox(stmt.GetValueDouble(5), stmt.GetValueDouble(6), stmt.GetValueDouble(7), stmt.GetValueDouble(8));

        Placement2d placement(origin, rotation, bbox);
        return TileGeometrySource2d::Create(categoryId, db, geomBlob, placement);
        }
};

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

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct TileContext : NullContext
{
    enum class Result { Success, NoGeometry, Aborted };
private:
    IFacetOptionsR                  m_facetOptions;
    RootR                           m_root;
    GeometryList&                   m_geometries;
    DRange3d                        m_range;
    DRange3d                        m_tileRange;
    BeSQLite::CachedStatementPtr    m_statement;
    LoadContext                     m_loadContext;
    Transform                       m_transformFromDgn;
    double                          m_minRangeDiagonalSquared;
    double                          m_minTextBoxSize;
    double                          m_tolerance;
    GraphicPtr                      m_finishedGraphic;
    TileBuilderPtr                  m_tileBuilder;
    TileSubGraphicPtr               m_subGraphic;
    DgnElementId                    m_curElemId;
    double                          m_curRangeDiagonalSquared;
protected:
    void PushGeometry(GeometryR geom);

    TileContext(GeometryList& geoms, RootR root, DRange3dCR range, IFacetOptionsR facetOptions, TransformCR transformFromDgn, double tileTolerance, double rangeTolerance, LoadContextCR loadContext);

    Render::GraphicBuilderPtr _CreateGraphic(Render::GraphicBuilder::CreateParams const& params) override
        {
        Render::GraphicBuilderPtr builder(m_tileBuilder);
        if (builder->GetRefCount() > 2)
            builder = new TileBuilder(*this, m_curElemId, m_curRangeDiagonalSquared, params);
        else
            m_tileBuilder->ReInitialize(m_curElemId, m_curRangeDiagonalSquared, params.GetPlacement());

        BeAssert(builder->GetRefCount() <= 2);
        return builder;
        }

    bool IsValueNull(int index) { return m_statement->IsColumnNull(index); }
    StatusInt _VisitElement(DgnElementId elementId, bool allowLoad) override;
    Render::GraphicPtr _StrokeGeometry(GeometrySourceCR, double) override;
    void _AddSubGraphic(Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, GeometryParamsR geomParams) override;
    bool _CheckStop() override { return WasAborted() || AddAbortTest(m_loadContext.WasAborted()); }
    bool _WantUndisplayed() override { return true; }
    AreaPatternTolerance _GetAreaPatternTolerance(CurveVectorCR) override { return AreaPatternTolerance(m_tolerance); }
    Render::SystemP _GetRenderSystem() const override { return m_loadContext.GetRenderSystem(); }
    double _GetPixelSizeAtPoint(DPoint3dCP) const override { return m_tolerance; }
    bool _WantGlyphBoxes(double sizeInPixels) const override { return wantGlyphBoxes(sizeInPixels); }
    void _DrawStyledCurveVector(GraphicBuilderR builder, CurveVectorCR curve, GeometryParamsR params, bool doCook) override
        {
        bool wasCurved = setupForStyledCurveVector(builder, curve);
        NullContext::_DrawStyledCurveVector(builder, curve, params, doCook);
        static_cast<TileBuilderR>(builder).SetAddingCurved(wasCurved);
        }
    bool _AnyPointVisible(DPoint3dCP pts, int nPts, double tolerance) override
        {
        DRange3d range = DRange3d::From(pts, nPts);
        return range.IntersectsWith(m_range);
        }
public:
    TileContext(GeometryList& geometries, RootR root, DRange3dCR range, IFacetOptionsR facetOptions, TransformCR transformFromDgn, double tolerance, LoadContextCR loadContext)
        : TileContext(geometries, root, range, facetOptions, transformFromDgn, tolerance, tolerance, loadContext) { }

    static Render::ViewFlags GetDefaultViewFlags();

    void ProcessElement(DgnElementId elementId, double diagonalRangeSquared);
    void AddGeomPart (Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, GeometryParamsR geomParams, GraphicParamsR graphicParams);
    GeomPartPtr GenerateGeomPart(DgnGeometryPartCR, GeometryParamsR);

    RootR GetRoot() const { return m_root; }
    System& GetRenderSystemR() const { BeAssert(nullptr != m_loadContext.GetRenderSystem()); return *m_loadContext.GetRenderSystem(); }
    bool Is3d() const { return m_root.Is3d(); }

    double GetMinRangeDiagonalSquared() const { return m_minRangeDiagonalSquared; }
    double GetTolerance() const { return m_tolerance; }
    bool BelowMinRange(DRange3dCR range) const
        {
        // Avoid processing any bits of geometry with range smaller than roughly half a pixel...
        auto diag = Is3d() ? range.low.DistanceSquared(range.high) : range.low.DistanceSquaredXY(range.high);
        return diag < m_minRangeDiagonalSquared && 0.0 < diag; // ###TODO_ELEMENT_TILE: Dumb single-point primitives...
        }

    size_t GetGeometryCount() const { return m_geometries.size(); }
    void TruncateGeometryList(size_t maxSize) { m_geometries.resize(maxSize); m_geometries.MarkIncomplete(); }


    DgnElementId GetCurrentElementId() const { return m_curElemId; }
    TransformCR GetTransformFromDgn() const { return m_transformFromDgn; }
    IFacetOptionsR GetFacetOptions() { return m_facetOptions; }
    LoadContextCR GetLoadContext() const { return m_loadContext; }

    GraphicPtr FinishGraphic(GeometryAccumulatorR accum, TileBuilder& builder);
    GraphicPtr FinishSubGraphic(GeometryAccumulatorR accum, TileSubGraphic& subGf);

    void MarkIncomplete() { m_geometries.MarkIncomplete(); }
    void SetLoadContext(LoadContextCR context) { m_loadContext = context; }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileBuilder::TileBuilder(TileContext& context, DgnElementId elemId, double rangeDiagonalSquared, CreateParams const& params)
    : GeometryListBuilder(context.GetRenderSystemR(), params, elemId, context.GetTransformFromDgn()), m_context(context), m_rangeDiagonalSquared(rangeDiagonalSquared)
    {
    SetCheckGlyphBoxes(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileBuilder::TileBuilder(TileContext& context, DRange3dCR range)
    : GeometryListBuilder(context.GetRenderSystemR(), CreateParams::Scene(context.GetDgnDb())), m_context(context),
    m_rangeDiagonalSquared(context.Is3d() ? range.low.DistanceSquared(range.high) : range.low.DistanceSquaredXY(range.high))
    {
    // for TileSubGraphic...
    SetCheckGlyphBoxes(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TileBuilder::ReInitialize(DgnElementId elemId, double rangeDiagonalSquared, TransformCR localToWorld)
    {
    GeometryListBuilder::ReInitialize(localToWorld, m_context.GetTransformFromDgn(), elemId);
    m_rangeDiagonalSquared = rangeDiagonalSquared;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TileBuilder::ReInitialize(DRange3dCR range)
    {
    GeometryListBuilder::ReInitialize(Transform::FromIdentity());
    m_rangeDiagonalSquared = m_context.Is3d() ? range.low.DistanceSquared(range.high) : range.low.DistanceSquaredXY(range.high);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TileBuilder::_AddPolyface(PolyfaceQueryCR geom, bool filled)
    {
    AddPolyfaceR(*geom.Clone(), filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TileBuilder::_AddPolyfaceR(PolyfaceHeaderR geom, bool filled)
    {
    Add(geom, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TileBuilder::_AddSubGraphic(GraphicR mainGraphic, TransformCR subToGraphic, GraphicParamsCR params, ClipVectorCP clip)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicBuilderPtr TileBuilder::_CreateSubGraphic(TransformCR tf, ClipVectorCP clip) const
    {
    CreateParams params = GetCreateParams().SubGraphic(Transform::FromProduct(GetLocalToWorldTransform(), tf));
    TileBuilderPtr subGf = new TileBuilder(m_context, GetElementId(), m_rangeDiagonalSquared, params);
    subGf->ActivateGraphicParams(GetGraphicParams(), GetGeometryParams());

    if (nullptr != clip)
        {
        ClipVectorPtr tClip = clip->Clone(&GetLocalToWorldTransform());

        Transform tileTransform;
        tileTransform.InverseOf(m_context.GetRoot().GetLocationForTileGeneration());
        tClip->TransformInPlace(tileTransform);

        subGf->SetCurrentClip(tClip.get());
        }

    return subGf.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicPtr TileBuilder::_FinishGraphic(GeometryAccumulatorR accum)
    {
    BeAssert(nullptr == GetGeometryParams() || GetGeometryParams()->GetSubCategoryId().IsValid());
    return m_context.FinishGraphic(accum, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileBuilder::_WantStrokeLineStyle(LineStyleSymbCR lsSymb, IFacetOptionsPtr& options)
    {
    if (!lsSymb.GetUseStroker())
        return false;

    // We need to stroke if either the stroke length or width exceeds tolerance...
    ILineStyleCP        lineStyle;
    double              maxDimension = (nullptr == (lineStyle = lsSymb.GetILineStyle())) ? 0.0 : lsSymb.GetScale() * std::max(lineStyle->GetMaxWidth(), lineStyle->GetLength());
    constexpr double    s_strokeLineStylePixels = 5.0;      // Stroke if max dimension exceeds 5 pixels.

    if (maxDimension > s_strokeLineStylePixels * m_context.GetTolerance())
        {
        options = &m_context.GetFacetOptions();
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileBuilder::_WantPreBakedBody(IBRepEntityCR body)
    {
    // Comment out the follow #define to test behavior of BReps on platforms which lack Parasolid.
// #define TEST_BREP_POLYFACES
#if defined (BENTLEYCONFIG_PARASOLID) && !defined(TEST_BREP_POLYFACES)
    // ###TODO: Take this tile's tolerance into account; also would be nice to detect single planar sheets since the BRepCurveVector should suffice even if curved.
    bool curved = BRepUtil::HasCurvedFaceOrEdge(body);
    return !curved;
#else
    return true;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileSubGraphic::TileSubGraphic(TileContext& context, DgnGeometryPartCP part)
    : TileBuilder(context, nullptr != part ? static_cast<DRange3d>(part->GetBoundingBox()) : DRange3d::NullRange()), m_input(part)
    {
    SetCheckGlyphBoxes(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TileSubGraphic::ReInitialize(DgnGeometryPartCR part)
    {
    TileBuilder::ReInitialize(part.GetBoundingBox());
    m_input = &part;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicPtr TileSubGraphic::_FinishGraphic(GeometryAccumulatorR accum)
    {
    return m_context.FinishSubGraphic(accum, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileContext::TileContext(GeometryList& geometries, RootR root, DRange3dCR range, IFacetOptionsR facetOptions, TransformCR transformFromDgn, double tileTolerance, double rangeTolerance, LoadContextCR loadContext)
  : m_geometries (geometries), m_facetOptions(facetOptions), m_root(root), m_range(range), m_transformFromDgn(transformFromDgn),
    m_tolerance(tileTolerance), m_statement(root.GetDgnDb().GetCachedStatement(root.Is3d() ? GeometrySelector3d::GetSql() : GeometrySelector2d::GetSql())),
    m_loadContext(loadContext), m_finishedGraphic(new Graphic(root.GetDgnDb()))
    {
    static const double s_minTextBoxSize = 1.0;     // Below this ratio to tolerance  text is rendered as box.

    m_minRangeDiagonalSquared = s_minRangeBoxSize * rangeTolerance;
    m_minRangeDiagonalSquared *= m_minRangeDiagonalSquared;
    m_minTextBoxSize  = s_minTextBoxSize * rangeTolerance;
    GetTransformFromDgn().Multiply (m_tileRange, m_range);

    SetDgnDb(root.GetDgnDb());
    m_is3dView = root.Is3d();
    SetViewFlags(GetDefaultViewFlags());

    // These are reused...
    m_tileBuilder = new TileBuilder(*this, DgnElementId(), 0.0, GraphicBuilder::CreateParams::Scene(root.GetDgnDb()));
    m_subGraphic = new TileSubGraphic(*this);
    }

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
    LoadContextCR   m_loadContext;
    DgnElementIdSet const*  m_skipElems;
    size_t          m_numSkipped = 0;
    bool            m_aborted = false;
    bool            m_is2d;

    bool CheckStop() { return m_aborted || (m_aborted = m_loadContext.WasAborted()); }
    bool SkipElem(DgnElementId elemId) const { return nullptr != m_skipElems && m_skipElems->end() != m_skipElems->find(elemId); }

    void Insert(double diagonalSq, DgnElementId elemId)
        {
        BeAssert(!SkipElem(elemId));
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
        else if (SkipElem(entry.m_id))
            return Stop::No;
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
    ElementCollector(DRange3dCR range, RangeIndex::Tree& rangeIndex, double minRangeDiagonalSquared, LoadContextCR loadContext, uint32_t maxElements, DgnElementIdSet const* skipElems)
        : m_range(range), m_minRangeDiagonalSquared(minRangeDiagonalSquared), m_maxElements(maxElements), m_loadContext(loadContext), m_is2d(!rangeIndex.Is3d()), m_skipElems(skipElems)
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

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Loader::Loader(TileR tile, TileTree::TileLoadStatePtr loads, Dgn::Render::SystemP renderSys)
    : T_Super("", tile, loads, tile.GetRoot()._ConstructTileResource(tile), renderSys), m_createTime(tile.GetElementRoot().GetModel()->GetLastElementModifiedTime()), m_cacheCreateTime(m_createTime),
    m_doTileRepair(T_HOST._IsFeatureEnabled("Platform.TileRepair"))
    {
#if defined(DISABLE_PARTIAL_TILES)
    if (nullptr != loads)
        loads->ClearPartialTimeout();
#else
    // We only create partial tiles for the 'root' tiles (the top-most displayable tiles) because they are the first tiles we generate for an empty view,
    // and can always be substituted while higher-resolution child tiles are being (fully) generated.
    if (nullptr != loads && loads->HasPartialTimeout() && tile.IsParentDisplayable())
        loads->ClearPartialTimeout();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> Loader::_GetFromSource()
    {
    LoaderPtr me(this);
    return folly::via(&BeFolly::ThreadPool::GetCpuPool(), [me]() { return me->DoGetFromSource(); });
    }


#ifdef TILECACHE_DEBUG
static double s_displayTime = 5.0;   // Every 5 second.

struct TileCacheStatistics
{
    size_t      m_emptyTileCount = 0;
    double      m_emptyTileTime = 0.0;
    size_t      m_totalTileCount = 0;
    double      m_totalTime = 0.0;
    double      m_lastDisplayTime = 0.0;
    double      m_totalReadTime = 0.0;
    StopWatch   m_stopWatch;
    BeMutex     m_mutex;

void    Update(double time, bool empty)
    {
    BeMutexHolder lock(m_mutex);

    m_totalTileCount++;
    m_totalTime += time;
    if (empty)
        {
        m_emptyTileCount++;
        m_emptyTileTime += time;
        }
    Display();
    }

void    UpdateRead(double time)
    {
    BeMutexHolder lock(m_mutex);

    m_totalReadTime  += time;
    Display();
    }

    
void Display()
    {
    if (m_stopWatch.GetCurrentSeconds() - m_lastDisplayTime > s_displayTime)
        {
        TILECACHE_PRINTF("Total: %d, Empty: %d: (%f %%) Empty Tile: %f (%f), Total: %f, Non Empty: %f, Read: %f", m_totalTileCount, m_emptyTileCount, 100.0 * (double) m_emptyTileCount / (double) m_totalTileCount, m_emptyTileTime, m_emptyTileTime / m_totalTime, m_totalTime, m_totalTime - m_emptyTileTime, m_totalReadTime);
        m_lastDisplayTime = m_stopWatch.GetCurrentSeconds();
        }
    }
};
static TileCacheStatistics       s_statistics;
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool Loader::IsCacheable() const
    {
    return GetElementTile().IsCacheable();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Tile::IsCacheable() const
    {
#if defined(DISABLE_TILE_CACHE)
    // Tile cache is really annoying when debugging tile generation code...
    return false;
#else
    // Host can specify no caching.
    if (!T_HOST.GetTileAdmin()._WantCachedTiles(GetRoot().GetDgnDb()))
        return false;

    // Don't cache tiles refined for zoom...
    if (HasZoomFactor() && GetZoomFactor() > 1.0)
        return T_HOST.GetTileAdmin()._WantCachedHiResTiles(GetRoot().GetDgnDb());

    return true;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> Loader::_ReadFromDb()
    {
    StopWatch   stopWatch(true);

    folly::Future<BentleyStatus> status = T_Super::_ReadFromDb();

#ifdef TILECACHE_DEBUG    
    s_statistics.UpdateRead(stopWatch.GetCurrent());
#endif
    return status;
    }

#if defined(DEBUG_TILE_CACHE_GEOMETRY)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T> static bool areEqual(T const& lhs, T const& rhs) { return lhs == rhs; }
template <> bool areEqual(DisplayParamsCR lhs, DisplayParamsCR rhs) { return lhs.IsEqualTo(rhs); }
template <> bool areEqual(MeshPolyline const& lhs, MeshPolyline const& rhs) { return areEqual(lhs.GetIndices(), rhs.GetIndices()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T> static bool areEqual(bvector<T> const& lhs, bvector<T> const& rhs)
    {
    if (lhs.size() != rhs.size())
        return false;

    for (size_t i = 0; i < lhs.size(); i++)
        if (!areEqual(lhs[i], rhs[i]))
            return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T, typename U> static bool areEqual(bmap<T, U> const& lhs, bmap<T, U> const& rhs)
    {
    if (lhs.size() != rhs.size())
        return false;

    for (auto const& kvp : lhs)
        {
        auto iter = rhs.find(kvp.first);
        if (rhs.end() == iter || !areEqual(kvp.second, iter->second))
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T> static bool assertEqual(T const& lhs, T const& rhs)
    {
    bool equal = areEqual(lhs, rhs);
    BeAssert(equal);
    return equal;
    }

#define ASSERT_EQ(LHS, RHS) assertEqual((LHS), (RHS))
#define ASSERT_EQ_MEMBER(MEMBER) ASSERT_EQ((lhs.MEMBER), (rhs.MEMBER))

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void assertEqual(MeshCR lhs, MeshCR rhs)
    {
    ASSERT_EQ_MEMBER(IsEmpty());
    ASSERT_EQ_MEMBER(Is2d());
    ASSERT_EQ_MEMBER(IsPlanar());
    ASSERT_EQ_MEMBER(GetType());
    ASSERT_EQ_MEMBER(GetDisplayParams());
    ASSERT_EQ_MEMBER(Triangles().Indices());
    ASSERT_EQ_MEMBER(Polylines());
    ASSERT_EQ_MEMBER(Points());

    // Normals may be generated during collection then discarded when written to cache
    if (!lhs.GetDisplayParams().IgnoresLighting())
        ASSERT_EQ_MEMBER(Normals());

    ASSERT_EQ_MEMBER(Params());
    ASSERT_EQ_MEMBER(Colors());
    ASSERT_EQ_MEMBER(GetColorTable().GetMap());

    //FeatureIndex lhsFeatures, rhsFeatures;
    //lhs.ToFeatureIndex(lhsFeatures);
    //rhs.ToFeatureIndex(rhsFeatures);
    //ASSERT_EQ(lhsFeatures, rhsFeatures);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void assertEqual(MeshListCR lhs, MeshListCR rhs)
    {
    if (lhs.size() != rhs.size())
        {
        BeAssert(false);
        return;
        }

    for (size_t i = 0; i < lhs.size(); i++)
        assertEqual(*lhs[i], *rhs[i]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void assertEqual(Render::Primitives::GeometryCollectionCR lhs, Render::Primitives::GeometryCollectionCR rhs)
    {
    ASSERT_EQ_MEMBER(IsEmpty());
    ASSERT_EQ_MEMBER(IsComplete());
    ASSERT_EQ_MEMBER(ContainsCurves());
    assertEqual(lhs.Meshes(), rhs.Meshes());
    }

#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Loader::_LoadTile() 
    { 
    TileR                                   tile = GetElementTile();
    RootR                                   root = tile.GetElementRoot();
    ElementAlignedBox3d                     contentRange;
    StopWatch                               stopWatch(true);

    bool isLeafInCache = false;
    auto& geometry = m_geometry;
    if (!IsCacheable())
        {
        if (SUCCESS != LoadGeometryFromModel())
            return ERROR;
        }
    else
        {
        // NB: If we loaded this tile from the cache, m_saveToCache will be false. Read it from the cache data.
        // Otherwise, we've already populated m_geometry from model and written it to m_tileBytes. Do not deserialize it again.
        if (!m_tileBytes.empty() && !m_saveToCache)
            {
            BeAssert(geometry.IsEmpty());
            if (TileTree::IO::ReadStatus::Success != TileTree::IO::ReadDgnTile (contentRange, geometry, m_tileBytes, *root.GetModel(), *GetRenderSystem(), isLeafInCache, tile.GetRange()))
                {
                BeAssert(false);
                return ERROR;
                }
            }

        tile.SetContentRange(contentRange);
        }

#ifdef TILECACHE_DEBUG
    s_statistics.Update(stopWatch.GetCurrentSeconds(), geometry.IsEmpty());
#endif

    // No point subdividing empty Tiles - improves performance if we don't
    // Also not much point subdividing nodes containing no curved geometry
    // NB: We cannot detect either of the above if any elements or geometry were skipped during tile generation.
    if (isLeafInCache || geometry.IsComplete())
        {
        // NB: Above a certain tolerance we must subdivide in order to avoid visible precision issues with quantized positions when zooming in.
        if (geometry.IsEmpty() || tile.GetTolerance() <= s_maxLeafTolerance)
            {
            if (geometry.IsEmpty() || !geometry.ContainsCurves())
                tile.SetIsLeaf();
            else if (isLeafInCache)
                tile.SetZoomFactor(1.0);
            }
        }

    auto  system = GetRenderSystem();
    if (nullptr == GetRenderSystem())
        {
        // This is checked in _CreateTileTree()...
        BeAssert(false && "ElementTileTree requires a Render::System");
        return ERROR;
        }

    MeshGraphicArgs args;
    bvector<Render::GraphicPtr> graphics;

    for (auto const& mesh : geometry.Meshes())
        {
        auto meshGraphic = mesh->GetGraphics(args, *system, root.GetDgnDb());
        if (meshGraphic.IsValid())
            graphics.push_back(meshGraphic);
        }

    GraphicPtr batch;
    if (!graphics.empty())
        {
        GraphicPtr graphic;
        switch (graphics.size())
            {
            case 0:
                break;
            case 1:
                graphic = *graphics.begin();
                BeAssert(graphic.IsValid());
                break;
            default:
                BeAssert(std::accumulate(graphics.begin(), graphics.end(), true, [](bool cur, GraphicPtr const& gf) { return cur && gf.IsValid(); }));
                graphic = system->_CreateGraphicList(std::move(graphics), root.GetDgnDb());
                BeAssert(graphic.IsValid());
                break;
            }

        if (graphic.IsValid())
            {
            geometry.Meshes().m_features.SetModelId(root.GetModelId());
            batch = system->_CreateBatch(*graphic, std::move(geometry.Meshes().m_features), tile._GetContentRange());
            BeAssert(batch.IsValid());
            tile.SetGraphic(*batch);
            }
        }

    return tile.GetElementRoot().UnderMutex([&]()
        {
        if (batch.IsValid())
            tile.SetGraphic(*batch);

        if (!tile._IsPartial())
            {
            // Possible that one or more parent tiles will contain solely elements too small to produce geometry, in which case
            // we must create their children in order to get graphics...mark undisplayable.
            tile.SetDisplayable(batch.IsValid());
            tile.ClearBackupGraphic();
            return SUCCESS;
            }
        else
            {
            // Mark partial tile as canceled so it becomes 'not loaded' again and we can resume tile generation from where we left off...
            BeAssert(nullptr != m_loads);
            m_loads->SetCanceled();

            // Also notify host that a new tile has become available, though it's only partial - otherwise it won't know to recreate the scene...
            T_HOST.GetTileAdmin()._OnNewTileReady(root.GetDgnDb());

            return ERROR;
            }
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Loader::DoGetFromSource()
    {
    if (!IsCacheable())
        return IsCanceledOrAbandoned() ? ERROR : SUCCESS;
      
    TileR   tile = GetElementTile();
    RootR   root = tile.GetElementRoot();

    if (SUCCESS != LoadGeometryFromModel())
        return ERROR;

    auto& geometry = m_geometry;
    if (geometry.IsEmpty() && geometry.IsComplete())
        {
        m_tileBytes.clear();
        }
    else
        {
        // TBD -- Avoid round trip through m_tileBytes when loading from elements.
        // NB: Tile may not be a leaf, but may have zoom factor of 1.0 indicating it should not be sub-divided
        // (it can be refined to higher zoom level - those tiles are not cached unless admin specifically requests so).
        BeAssert(!tile.HasZoomFactor() || 1.0 == tile.GetZoomFactor() || T_HOST.GetTileAdmin()._WantCachedHiResTiles(root.GetDgnDb()));
        bool isLeaf = tile.IsLeaf() || tile.HasZoomFactor();
        if (SUCCESS != TileTree::IO::WriteDgnTile (m_tileBytes, tile._GetContentRange(), geometry, *root.GetModel(), isLeaf))
            return ERROR;

#if defined(DEBUG_TILE_CACHE_GEOMETRY)
        ElementAlignedBox3d readRange;
        Render::Primitives::GeometryCollection readGeometry;
        bool readIsLeaf;
        m_tileBytes.ResetPos();
        if (TileTree::IO::ReadStatus::Success != TileTree::IO::ReadDgnTile(readRange, readGeometry, m_tileBytes, *root.GetModel(), *GetRenderSystem(), readIsLeaf))
            BeAssert(false);

        assertEqual(readGeometry, geometry);
        m_tileBytes.ResetPos();
#endif
        }
    
    m_saveToCache = true;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool Loader::_IsExpired(uint64_t createTimeMillis)
    {
    m_cacheCreateTime = createTimeMillis;
    if (m_doTileRepair)
        return false;

    auto& tile = GetElementTile();
    uint64_t lastModMillis = tile.GetElementRoot().GetModel()->GetLastElementModifiedTime();
    return createTimeMillis < static_cast<uint64_t>(lastModMillis);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Loader::_IsValidData()
    {
    BeAssert(!m_tileBytes.empty());
    TileTree::IO::DgnTileReader reader(m_tileBytes, *GetElementTile().GetElementRoot().GetModel(), *GetRenderSystem());

    if (!m_doTileRepair)
        return reader.VerifyFeatureTable();

    if (IsExpired())
        reader.GetElements(m_omitElemIds, m_tileElemIds, m_cacheCreateTime);
    else
        reader.FindDeletedElements(m_omitElemIds);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Loader::_WantWaitOnSave() const
    {
    // BuildingConceptStation's tools invalidate tiles on every single mouse motion.
    // If we don't wait on the save to cache, then a subsequent tile invalidation may cause cache row to be updated/deleted
    // before a pending save completes, producing errors in sqlite.
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Loader::_IsCompleteData()
    {
    if (!m_doTileRepair)
        return true;

#if defined(TEST_TILE_REPAIR)
    return false;
#else
    return !IsExpired() && m_omitElemIds.empty();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileCR Loader::GetElementTile() const { return static_cast<TileCR>(*m_tile); }
TileR Loader::GetElementTile() { return static_cast<TileR>(*m_tile); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Root::Root(GeometricModelR model, TransformCR transform, Render::SystemR system) : T_Super(model, transform, "", &system),
    m_name(model.GetName()),
#if defined(CACHE_LARGE_GEOMETRY)
    m_cacheGeometry(m_is3d)
#else
    m_cacheGeometry(false)
#endif
    {
    // ###TODO: Play with this? Default of 20 seconds is ok for reality tiles which are cached...pretty short for element tiles.
    SetExpirationTime(BeDuration::Seconds(90));

    m_cache = model.GetDgnDb().ElementTileCache();
    }

#if defined (BENTLEYCONFIG_PARASOLID) 
static RefCountedPtr<PSolidThreadUtil::MainThreadMark> s_psolidMainThreadMark;
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
RootPtr Root::Create(GeometricModelR model, Render::SystemR system)
    {
    // DgnDb::VerifyClientThread(); ###TODO: Relax this constraint when publishing view attachments to Cesium...

    if (DgnDbStatus::Success != model.FillRangeIndex())
        return nullptr;

    DRange3d range;
    bool populateRootTile;
    if (model.Is3dModel())
        {
        range = model.GetDgnDb().GeoLocation().GetProjectExtents();
        range = scaleSpatialRange(range);
        populateRootTile = isElementCountLessThan(s_minElementsPerTile, *model.GetRangeIndex());
        }
    else
        {
        RangeAccumulator accum(range, model.Is2dModel());
        if (!accum.Accumulate(*model.GetRangeIndex()))
            range = DRange3d::From(DPoint3d::FromZero());

        auto sheet = model.ToSheetModel();
        if (nullptr != sheet)
            range.Extend(sheet->GetSheetExtents());

        populateRootTile = accum.GetElementCount() < s_minElementsPerTile;
        }

#if defined(POPULATE_ROOT_TILE)
    // For debugging...
    populateRootTile = true;
#endif

    // Translate world coordinates to center of range in order to reduce precision errors
    DPoint3d centroid = DPoint3d::FromInterpolate(range.low, 0.5, range.high);
    Transform transform = Transform::From(centroid);

#if defined (BENTLEYCONFIG_PARASOLID)
    PSolidKernelManager::StartSession();

    if (s_psolidMainThreadMark.IsNull())
        s_psolidMainThreadMark = new PSolidThreadUtil::MainThreadMark();
#endif

    RootPtr root = new Root(model, transform, system);
    Transform rangeTransform;

    rangeTransform.InverseOf(transform);
    DRange3d tileRange;
    rangeTransform.Multiply(tileRange, range);
    return root->LoadRootTile(tileRange, model, populateRootTile) ? root : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Root::LoadRootTile(DRange3dCR range, GeometricModelR model, bool populate)
    {
    // We want to generate the lowest-resolution tiles before any of their descendants, so that we always have *something* to draw
    // However, if we generated the single root tile before any others, we'd have to process every element in the model and waste all our work threads.
    // Instead, make the root tile empty & undisplayable; its direct children can be generated in parallel instead as the lowest-resolution tiles.
    // Optimization: Don't do this if the number of elements in the model is less than the min number of elements per tile, so that we reduce the number
    // of tiles required.
    m_rootTile = Tile::CreateRoot(*this, range, populate);

    if (!populate)
        m_rootTile->SetIsReady();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
GeomPartPtr Root::FindOrInsertGeomPart(DgnGeometryPartId partId, Render::GeometryParamsR geomParams, ViewContextR context)
    {
    return m_geomParts.FindOrInsert(partId, GetDgnDb(), geomParams, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
GeomPartPtr GeomPartCache::FindOrInsert(DgnGeometryPartId partId, DgnDbR db, Render::GeometryParamsR geomParams, ViewContextR context)
    {
#if defined(SHARE_GEOMETRY_PARTS)
    m_mutex.lock(); // << LOCK

    auto foundPart = m_parts.find(partId);
    if (m_parts.end() != foundPart)
        {
        // This part has already been created and cached.
        m_mutex.unlock(); // >> UNLOCK
        return foundPart->second;
        }

    auto foundBuilder = m_builders.find(partId);
    if (m_builders.end() != foundBuilder)
        {
        // Another thread is currently generating this part. Wait for it to finish.
        GeomPartBuilderPtr builder = foundBuilder->second;
        m_mutex.unlock(); // >> UNLOCK
        return builder->WaitForPart();
        }

    // We need to create this part. Any other threads that also want this part should wait while we create it.
    GeomPartBuilderPtr builder = GeomPartBuilder::Create();
    m_builders.Insert(partId, builder);
    builder->GetMutex().lock();
    m_mutex.unlock(); // >> UNLOCK

    GeomPartPtr part = builder->GeneratePart(partId, db, geomParams, context);

    // NB: Mark as "cached" even if cache disabled - waiting threads may end up using it too
    if (part.IsValid())
        part->SetInCache(true);

    m_mutex.lock(); // << LOCK

#if defined CACHE_GEOMETRY_PARTS
    BeAssert(m_parts.end() == m_parts.find(partId));
    m_parts.Insert(partId, part);
#endif

    foundBuilder = m_builders.find(partId);
    BeAssert(m_builders.end() != foundBuilder);
    m_builders.erase(foundBuilder);

    m_mutex.unlock(); // >> UNLOCK
    builder->GetMutex().unlock();
    builder->NotifyAll();

    return part;
#else
    GeomPartBuilderPtr builder = GeomPartBuilder::Create();
    GeomPartPtr part = builder->GeneratePart(partId, db, geomParams, context);
    return part;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
GeomPartPtr GeomPartBuilder::WaitForPart()
    {
    BeMutexHolder lock(GetMutex());
    while (m_part.IsNull())
        m_cv.InfiniteWait(lock);

    BeAssert(m_part.IsValid());
    return m_part;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
GeomPartPtr GeomPartBuilder::GeneratePart(DgnGeometryPartId partId, DgnDbR db, Render::GeometryParamsR geomParams, ViewContextR vc)
    {
    BeAssert(m_part.IsNull());
    BeAssert(nullptr != dynamic_cast<TileContext*>(&vc));

    auto& context = static_cast<TileContext&>(vc);
    DgnGeometryPartCPtr geomPart = db.Elements().Get<DgnGeometryPart>(partId);
    if (geomPart.IsNull())
        return nullptr;

    m_part = context.GenerateGeomPart(*geomPart, geomParams);
    return m_part;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
GeomPartPtr TileContext::GenerateGeomPart(DgnGeometryPartCR geomPart, GeometryParamsR geomParams)
    {
    TileSubGraphicPtr partBuilder(m_subGraphic);
    if (partBuilder->GetRefCount() > 2)
        partBuilder = new TileSubGraphic(*this, geomPart);
    else
        partBuilder->ReInitialize(geomPart);

    GeometryStreamIO::Collection collection(geomPart.GetGeometryStream().GetData(), geomPart.GetGeometryStream().GetSize());
    collection.Draw(*partBuilder, *this, geomParams, false, &geomPart);

    partBuilder->Finish();
    BeAssert(partBuilder->GetRefCount() <= 2);
    return partBuilder->GetOutput();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TileContext::AddGeomPart(Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, GeometryParamsR geomParams, GraphicParamsR graphicParams)
    {
    GeomPartPtr tileGeomPart = m_root.FindOrInsertGeomPart(partId, geomParams, *this);
    if (tileGeomPart.IsNull())
        return;

    DRange3d range;
    Transform partToWorld = Transform::FromProduct(graphic.GetLocalToWorldTransform(), subToGraphic);
    Transform tf = Transform::FromProduct(GetTransformFromDgn(), partToWorld);
    tf.Multiply(range, tileGeomPart->GetRange());

    DisplayParamsCR displayParams = m_tileBuilder->GetDisplayParamsCache().GetForMesh(graphicParams, &geomParams, false);

    BeAssert(nullptr != dynamic_cast<TileBuilderP>(&graphic));
    auto& parent = static_cast<TileBuilderR>(graphic);
    parent.Add(*Geometry::Create(*tileGeomPart, tf, range, GetCurrentElementId(), displayParams, GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
GeomPartPtr Root::FindOrInsertGeomPart(ISolidPrimitiveR prim, DRange3dCR range, DisplayParamsCR displayParams, DgnElementId elemId) const
    {
    BeMutexHolder lock(m_mutex);
    return m_solidPrimitiveParts.FindOrInsert(prim, range, displayParams, elemId, GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
GeomPartPtr Root::SolidPrimitivePartMap::FindOrInsert(ISolidPrimitiveR prim, DRange3dCR range, DisplayParamsCR displayParams, DgnElementId elemId, DgnDbR db)
    {
    Key key(prim, range, displayParams);

    auto findRange = m_map.equal_range(key);
    for (auto curr = findRange.first; curr != findRange.second; ++curr)
        {
        if (curr->first.IsEqual(key))
            return curr->second;
        }

    IGeometryPtr geom = IGeometry::Create(&prim);
    GeometryList geomList;
    geomList.push_back(*Geometry::Create(*geom, Transform::FromIdentity(), range, elemId, displayParams, prim.HasCurvedFaceOrEdge(), db, false));
    auto part = GeomPart::Create(range, geomList);
    m_map.Insert(key, part);

    return part;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Root::SolidPrimitivePartMap::Key::operator<(Key const& rhs) const
    {
    double const* a1 = &m_range.low.x,
                * a2 = &rhs.m_range.low.x;

    double tolerance = s_solidPrimitivePartCompareTolerance;
    for (size_t i = 0; i < 6; i++)
        {
        if (*a1 < *a2 - tolerance)
            return true;
        else if (*a1 > *a2 + tolerance)
            return false;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Root::SolidPrimitivePartMap::Key::IsEqual(Key const& rhs) const
    {
    return m_displayParams->IsEqualTo(*rhs.m_displayParams, DisplayParams::ComparePurpose::Merge)
        && m_solidPrimitive->IsSameStructureAndGeometry(*rhs.m_solidPrimitive, s_solidPrimitivePartCompareTolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool Root::WantCacheGeometry(double rangeDiagSq) const
    {
    if (!m_cacheGeometry)
        return false;

    // Only cache geometry which occupies a significant portion of the model's range, since it will appear in many tiles
    constexpr double rangeRatio = 0.25;
    DRange3d range = ComputeRange();
    double diag = Is3d() ? range.low.DistanceSquared(range.high) : range.low.DistanceSquaredXY(range.high);
    if (0.0 == diag)
        return false;

    BeAssert(Is3d()); // we only bother caching for 3d...want rangeRatio relative to actual range, not expanded range
    return rangeDiagSq / diag >= rangeRatio;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::AddCachedGeometry(GeometryList const& geometry, size_t startIndex, DgnElementId elementId, double rangeDiagSq) const
    {
    if (!WantCacheGeometry(rangeDiagSq))
        return;

    BeMutexHolder lock(m_mutex);
    auto pair = m_geomLists.Insert(elementId, geometry.Slice(startIndex, geometry.size()));
    if (pair.second)
        {
        for (auto& geom : pair.first->second)
            geom->SetInCache(true);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool Root::GetCachedGeometry(GeometryList& geometry, DgnElementId elementId, double rangeDiagSq) const
    {
    // NB: Check the range so that we don't acquire the mutex for geometry too small to be in cache
    if (!WantCacheGeometry(rangeDiagSq))
        return false;

    BeMutexHolder lock(m_mutex);
    auto iter = m_geomLists.find(elementId);
    if (m_geomLists.end() == iter)
        return false;

    if (geometry.empty())
        geometry = iter->second;
    else
        geometry.append(iter->second);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::RemoveCachedGeometry(DRange3dCR range, DgnElementId id)
    {
    double rangeDiagSq = Is3d() ? range.low.DistanceSquared(range.high) : range.low.DistanceSquaredXY(range.high);
    if (WantCacheGeometry(rangeDiagSq))
        {
        BeMutexHolder lock(m_mutex);
        m_geomLists.erase(id);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
Root::DebugOptions Root::GetDebugOptions() const
    {
    return m_debugOptions | s_globalDebugOptions;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::ToggleDebugBoundingVolumes()
    {
    switch (s_globalDebugOptions)
        {
        case DebugOptions::None:
            s_globalDebugOptions = DebugOptions::ShowBoundingVolume;
            break;
        case DebugOptions::ShowContentVolume:
            s_globalDebugOptions = DebugOptions::None;
            break;
        default:
            s_globalDebugOptions = DebugOptions::ShowContentVolume;
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicPtr Tile::GetDebugGraphics(Root::DebugOptions options) const
    {
    if (!_HasGraphics())
        return nullptr;
    else if (m_debugGraphics.IsUsable(options))
        return m_debugGraphics.m_graphic;

    m_debugGraphics.m_options = options;

    bool wantRange = Root::DebugOptions::None != (Root::DebugOptions::ShowBoundingVolume & options);
    bool wantContentRange = Root::DebugOptions::None != (Root::DebugOptions::ShowContentVolume & options);
    if (!wantRange && !wantContentRange)
        return (m_debugGraphics.m_graphic = nullptr);

    GraphicBuilderPtr gf = GetElementRoot().GetRenderSystemP()->_CreateGraphic(GraphicBuilder::CreateParams::Scene(GetElementRoot().GetDgnDb()));
    GraphicParams params;
    params.SetWidth(0);
    if (wantRange)
        {
        ColorDef color = IsLeaf() ? ColorDef::DarkBlue() : (HasZoomFactor() ? ColorDef::DarkMagenta() : ColorDef::DarkOrange());
        params.SetLineColor(color);
        params.SetFillColor(color);
        params.SetLinePixels((IsLeaf() || HasZoomFactor()) ? LinePixels::Code5 : LinePixels::Code4);
        gf->ActivateGraphicParams(params);
        gf->AddRangeBox(GetRange());
        }

    if (wantContentRange)
        {
        params.SetLineColor(ColorDef::DarkRed());
        params.SetFillColor(ColorDef::DarkRed());
        params.SetLinePixels(LinePixels::Solid);
        gf->ActivateGraphicParams(params);
        gf->AddRangeBox(_GetContentRange());
        }

    m_debugGraphics.m_graphic = gf->Finish();
    return m_debugGraphics.m_graphic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Tile::_GetTileCacheKey() const
    {
    return GetElementRoot().GetModelId().ToString() + GetIdString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Tile::_DrawGraphics(TileTree::DrawArgsR args) const
    {
    GetElementRoot().UnderMutex([&]()
        {
        if (m_graphic.IsValid())
            args.m_graphics.Add(*m_graphic);
        else if (m_backupGraphic.IsValid())
            args.m_graphics.Add(*m_backupGraphic);
        });

    auto debugGraphic = GetDebugGraphics(GetElementRoot().GetDebugOptions());
    if (debugGraphic.IsValid())
        args.m_graphics.Add(*debugGraphic);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::Tile(Root& octRoot, TileTree::OctTree::TileId id, Tile const* parent, DRange3dCP range, bool displayable)
    : T_Super(octRoot, id, parent, false), m_displayable(displayable)
    {
    if (nullptr != parent)
        m_range = ElementAlignedBox3d(parent->ComputeChildRange(*this, octRoot.Is2d()));
    else
        m_range.Extend(*range);

    InitTolerance(s_minToleranceRatio);
    m_debugId = (octRoot.GetModelId().GetValue() << 32) + (id.m_level << 24) + (id.m_i << 16) + (id.m_j << 8) + id.m_k;
    }

/*---------------------------------------------------------------------------------**//**
* NB: Constructor used by ThumbnailTile...
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::Tile(Root& root, TileTree::OctTree::TileId id, DRange3dCR range, double minToleranceRatio)
    : T_Super(root, id, nullptr, true), m_displayable(true)
    {
    m_range.Extend(range);
    InitTolerance(minToleranceRatio, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::Tile(Tile const& parent) : T_Super(const_cast<Root&>(parent.GetElementRoot()), parent.GetTileId(), &parent, false)
    {
    m_range.Extend(parent.GetRange());

    BeAssert(parent.HasZoomFactor());
    SetZoomFactor(parent.GetZoomFactor() * 2.0);

    InitTolerance(s_minToleranceRatio);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Tile::InitTolerance(double minToleranceRatio, bool isLeaf)
    {
    double diagDist = GetElementRoot().Is3d() ? m_range.DiagonalDistance() : m_range.DiagonalDistanceXY();
    m_tolerance = diagDist / (minToleranceRatio * m_zoomFactor);
    m_isLeaf = isLeaf;
    BeAssert(0.0 != m_tolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Tile::_Invalidate()
    {
    GetElementRoot().UnderMutex([&]()
        {
        m_backupGraphic = m_graphic;
        m_graphic = nullptr;
        m_debugGraphics.Reset();
        m_generator.reset();

        m_contentRange = ElementAlignedBox3d();

        if (m_hasZoomFactor)
            _UnloadChildren(BeTimePoint::Now());

        m_isLeaf = false;
        m_hasZoomFactor = false;
        m_zoomFactor = 1.0;

        InitTolerance(s_minToleranceRatio);

        if (nullptr != GetParent())
            return;

        // Root tile...
        GeometricModelPtr model = GetElementRoot().GetModel();
        m_displayable = isElementCountLessThan(s_minElementsPerTile, *model->GetRangeIndex());
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool Tile::_IsInvalidated(TileTree::DirtyRangesCR dirty) const
    {
    if (IsLeaf())
        return true;

    double minRangeDiagonalSquared = s_minRangeBoxSize * m_tolerance;
    minRangeDiagonalSquared *= minRangeDiagonalSquared;
    for (DRange3dCR range : dirty)
        {
        double diagSq = GetElementRoot().Is3d() ? range.low.DistanceSquared(range.high) : range.low.DistanceSquaredXY(range.high);
        if (diagSq >= minRangeDiagonalSquared || diagSq == 0.0) // ###TODO_ELEMENT_TILE: Dumb single-point primitives...
            return true;
        }

    // No damaged range is large enough to contribute to this tile, so no need to regenerate it.
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void updateLowerBound(double& myValue, double parentOldValue, double parentNewValue, bool allowShrink)
    {
    if (DoubleOps::AlmostEqual(myValue, parentOldValue) && (allowShrink || parentOldValue > parentNewValue))
        myValue = parentNewValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void updateUpperBound(double& myValue, double parentOldValue, double parentNewValue, bool allowShrink)
    {
    if (DoubleOps::AlmostEqual(myValue, parentOldValue) && (allowShrink || parentOldValue < parentNewValue))
        myValue = parentNewValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::_OnProjectExtentsChanged(AxisAlignedBox3dCR newExtents)
    {
    // The range of a spatial tile tree == the project extents.
    // Note that currently we consider drawing outside of the project extents to be illegal.
    // Therefore we do not attempt to regenerate tiles to include geometry previously outside the extents, or exclude geometry previously within them.
    auto rootTile = static_cast<TileP>(GetRootTile().get());
    if (Is3d() && nullptr != rootTile && !m_ignoreChanges)
        {
        // ###TODO: What about non-spatial 3d models?
        Transform tfToTile;
        tfToTile.InverseOf(GetLocation());

        DRange3d tileRange;
        tfToTile.Multiply(tileRange, newExtents);

        rootTile->UpdateRange(rootTile->GetRange(), tileRange, true);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Tile::_UpdateRange(DRange3dCR parentOld, DRange3dCR parentNew)
    {
    // The range of a 2d tile tree == the range of geometry within the model.
    // This can change whenever elements are added/removed/modified.
    // Must update ranges of tiles to reflect change.
    // Invalidating tiles for modified geometry is handled by _Invalidate()
    if (!GetElementRoot().Is3d())
        {
        // ###TODO: What about non-spatial 3d models?
        UpdateRange(parentOld, parentNew, false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Tile::UpdateRange(DRange3dCR parentOld, DRange3dCR parentNew, bool allowShrink)
    {
    // Expand outside bounds of range to match the new range
    // NB: It doesn't matter if the new range intersects the tile's range - may still need to expand
    DRange3d myOld = GetRange();
    if (nullptr == GetParent())
        {
        if (allowShrink)
            m_range = ElementAlignedBox3d(parentNew);
        else
            m_range.UnionOf(parentOld, parentNew);
        }
    else
        {
        updateLowerBound(m_range.low.x, parentOld.low.x, parentNew.low.x, allowShrink);
        updateLowerBound(m_range.low.y, parentOld.low.y, parentNew.low.y, allowShrink);
        updateLowerBound(m_range.low.z, parentOld.low.z, parentNew.low.z, allowShrink);
        updateUpperBound(m_range.high.x, parentOld.high.x, parentNew.high.x, allowShrink);
        updateUpperBound(m_range.high.y, parentOld.high.y, parentNew.high.y, allowShrink);
        updateUpperBound(m_range.high.z, parentOld.high.z, parentNew.high.z, allowShrink);
        }

    auto children = _GetChildren(false);
    if (nullptr != children)
        {
        for (auto& child : *children)
            static_cast<TileP>(child.get())->UpdateRange(myOld, GetRange(), allowShrink);
        }

    m_debugGraphics.Reset(); // so we changes to bounding volumes immediately...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::TileLoaderPtr Tile::_CreateTileLoader(TileTree::TileLoadStatePtr loads, Dgn::Render::SystemP renderSys)
    {
    return Loader::Create(*this, loads, renderSys);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::TilePtr Tile::_CreateChild(TileTree::OctTree::TileId childId) const
    {
    return Tile::Create(const_cast<RootR>(GetElementRoot()), childId, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::ChildTiles const* Tile::_GetChildren(bool load) const
    {
    if (HasZoomFactor() && load && m_children.empty())
        {
        // Create a single child containing same geometry in same range, faceted to a higher resolution.
        m_children.push_back(CreateWithZoomFactor(*this));
        }

    return T_Super::_GetChildren(load);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Tile::_ValidateChildren() const
    {
    // Until the tile is ready, we won't know if it will be a leaf, be sub-divided, or have a zoom factor applied
    if (!IsReady() || !IsDisplayable())
        return;

    if (HasZoomFactor())
        {
        switch (m_children.size())
            {
            case 0:
                break;
            case 1:
                {
                // Child had zoom factor then was invalidated.
                auto child = static_cast<TileP>(m_children[0].get());
                if (!child->HasZoomFactor())
                    {
                    child->SetIsLeaf(false);
                    child->SetZoomFactor(2.0 * GetZoomFactor());
                    child->InitTolerance(s_minToleranceRatio);
                    }

                break;
                }
            default:
                // We previously sub-divided, now don't want to.
                _UnloadChildren(BeTimePoint::Now());
                break;
            }
        }
    else if (IsLeaf() || 1 == m_children.size())
        {
        // Child had zoom factor, now we no longer have it - may want to subdivide, or may have become a leaf
        _UnloadChildren(BeTimePoint::Now());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
double Tile::_GetMaximumSize() const
    {
    // returning 0.0 signifies undisplayable tile...
    return m_displayable ? s_tileScreenSize * m_zoomFactor : 0.0;
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
    friend struct GlyphDeferralManager;

private:
    TileCR                  m_tile;
    GeometryOptions         m_options;
    double                  m_tolerance;
    LoadContext             m_loadContext;
    size_t                  m_geometryCount = 0;
    FeatureTable            m_featureTable;
    MeshBuilderMap          m_builderMap;
    DRange3d                m_contentRange = DRange3d::NullRange();
    bool                    m_maxGeometryCountExceeded = false;
    bool                    m_didDecimate = false;
    ThematicMeshBuilderPtr  m_thematicMeshBuilder;
    GlyphDeferralManager    m_glyphDeferralManager;

    // ###TODO: Revisit...lots of small polyfaces can produce excessive vertices...
    static constexpr size_t GetDecimatePolyfacePointCount() { return 100; }

    MeshBuilderR GetMeshBuilder(MeshBuilderMap::Key const& key);
    DgnElementId GetElementId(GeometryR geom) const { return m_maxGeometryCountExceeded ? DgnElementId() : geom.GetEntityId(); }

    void AddPolyfaces(GeometryR geom, double rangePixels, bool isContained);
    void AddPolyfaces(PolyfaceList& polyfaces, GeometryR geom, double rangePixels, bool isContained);
    void AddPolyface(Polyface& polyface, GeometryR, double rangePixels, bool isContained, bool doDefer=true);
    void AddClippedPolyface(PolyfaceQueryCR, DgnElementId, DisplayParamsCR, MeshEdgeCreationOptions, bool isPlanar);

    void AddStrokes(GeometryR geom, double rangePixels, bool isContained);
    void AddStrokes(StrokesList& strokes, GeometryR geom, double rangePixels, bool isContained);
    void AddStrokes(StrokesR strokes, GeometryR geom, double rangePixels, bool isContained);
    Strokes ClipSegments(StrokesCR strokes) const;
    void ClipStrokes(StrokesR strokes) const;
    void ClipPoints(StrokesR strokes) const;

    SystemP _GetRenderSystem() const override { return m_loadContext.GetRenderSystem(); }
    GraphicBuilderPtr _CreateGraphic(GraphicBuilder::CreateParams const&) override { BeAssert(false); return nullptr; }
    GraphicPtr _CreateBranch(GraphicBranch&, DgnDbR, TransformCR, ClipVectorCP) override { BeAssert(false); return nullptr; }
    double _GetPixelSizeAtPoint(DPoint3dCP) const override { return m_tolerance; }
    bool _WantGlyphBoxes(double sizeInPixels) const override { return wantGlyphBoxes(sizeInPixels); }
    void _DrawStyledCurveVector(GraphicBuilderR builder, CurveVectorCR curve, GeometryParamsR params, bool doCook) override
        {
        bool wasCurved = setupForStyledCurveVector(builder, curve);
        ViewContext::_DrawStyledCurveVector(builder, curve, params, doCook);
        static_cast<TileBuilderR>(builder).SetAddingCurved(wasCurved);
        }
public:
    MeshGenerator(TileCR tile, GeometryOptionsCR options, LoadContextCR loadContext);

    bool DidDecimation() const { return m_didDecimate; }

    // Add meshes to the MeshBuilder map
    void AddMeshes(GeometryList const& geometries, bool doRangeTest);
    void AddMeshes(GeometryR geom, bool doRangeTest);
    void AddMeshes(GeomPartR part, bvector<GeometryCP> const& instances);
    void AddDeferredGlyphMeshes();

    // Return a list of all meshes currently in the builder map
    MeshList GetMeshes(bool isPartialTile);
    // Return a tight bounding volume
    DRange3dCR GetContentRange() const { return m_contentRange; }
    DRange3dCR GetTileRange() const { return m_builderMap.GetRange(); }
    TileCR GetTile() const { return m_tile; }
    void SetLoadContext(LoadContextCR context) { m_loadContext = context; }

    bool ReadFrom(TileTree::StreamBufferR, bool& containsCurves, bool& isIncomplete, DgnElementIdSet const& omitElems);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _AnyPointVisible(DPoint3dCP worldPoints, int nPts, double tolerance) override
    {
    DRange3d        pointRange = DRange3d::From(worldPoints, nPts);

    return pointRange.IntersectsWith(m_tile.GetRange());
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
MeshGenerator::MeshGenerator(TileCR tile, GeometryOptionsCR options, LoadContextCR loadContext)
  : m_tile(tile), m_options(options), m_tolerance(tile.GetTolerance()), m_loadContext(loadContext),
    m_featureTable(tile.GetElementRoot().GetModelId(), nullptr != loadContext.GetRenderSystem() ? loadContext.GetRenderSystem()->_GetMaxFeaturesPerBatch() : s_hardMaxFeaturesPerTile),
    m_builderMap(m_tolerance, &m_featureTable, tile.GetTileRange(), m_tile.GetElementRoot().Is2d())
    {
    SetDgnDb(m_tile.GetElementRoot().GetDgnDb());
    m_is3dView = m_tile.GetElementRoot().Is3d();
    SetViewFlags(TileContext::GetDefaultViewFlags());


    // For now always create -- (use first aux channel) - TBD control from UX.
    m_thematicMeshBuilder  = new ThematicMeshBuilder("", "");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
MeshBuilderR MeshGenerator::GetMeshBuilder(MeshBuilderMap::Key const& key)
    {
    return m_builderMap[key];
    }

// #define DEBUG_DUMP_TEXTURE_ATLAS_DIR L"E:\\texture_atlas\\"
#if defined(DEBUG_DUMP_TEXTURE_ATLAS_DIR)
static void writeAtlasToImageFile(Byte const* data, uint32_t width, uint32_t height, int32_t bytesPerPixel)
    {
    bool            rgba = 4 == bytesPerPixel;
    Image           image(width, height, ByteStream(data, width * height * bytesPerPixel), (rgba) ? Image::Format::Rgba : Image::Format::Rgb);
    ImageSource     imageSource(image, rgba ? ImageSource::Format::Png: ImageSource::Format::Jpeg);

    static BeAtomic<uint32_t> s_atlasIndex;
    WPrintfString filename(L"%lsatlas_%u.png", DEBUG_DUMP_TEXTURE_ATLAS_DIR, s_atlasIndex.IncrementAtomicPre());
    BeFile file;
    file.Create(filename.c_str(), true);
    file.Write(nullptr, imageSource.GetByteStream().GetData(), imageSource.GetByteStream().GetSize());
    file.Close();
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void* DeferredGlyph::GetKey()
    {
    return static_cast<void*>(m_polyface.m_glyphImage);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void GlyphAtlas::CalculateSize()
    {
    m_numGlyphsInX = m_numGlyphs >= m_maxAtlasGlyphsDim ? m_maxAtlasGlyphsDim : m_numGlyphs;
    if (!GlyphAtlas::IsPowerOfTwo(m_numGlyphsInX))
        m_numGlyphsInX = GlyphAtlas::NextHighestPowerOfTwo(m_numGlyphsInX);

    m_numGlyphsInY = static_cast<uint32_t>(ceil(static_cast<double>(m_numGlyphs) / m_maxAtlasGlyphsDim));
    if (!GlyphAtlas::IsPowerOfTwo(m_numGlyphsInY))
        m_numGlyphsInY = GlyphAtlas::NextHighestPowerOfTwo(m_numGlyphsInY);

    m_numPixelsInX = m_numGlyphsInX * m_glyphTotalSizeInPixels;
    m_numPixelsInY = m_numGlyphsInY * m_glyphTotalSizeInPixels;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
GlyphAtlas::GlyphAtlas(uint32_t index, uint32_t numGlyphs) : m_index(index), m_numGlyphs(numGlyphs)
    {
    CalculateSize();
    m_atlasBytes.Resize(m_numPixelsInX * m_numPixelsInY * 4); // reserve space for all potential glyph
    memset(m_atlasBytes.GetDataP(), 0, m_atlasBytes.GetSize());
    m_availGlyphSlotX = m_availGlyphSlotY = m_glyphHalfPaddingInPixels;
    m_atlasTexture = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
GlyphAtlas::GlyphAtlas(GlyphAtlas&& other)
    {
    m_index = other.m_index;
    m_numGlyphs = other.m_numGlyphs;
    m_numGlyphsInX = other.m_numGlyphsInX;
    m_numGlyphsInY = other.m_numGlyphsInY;
    m_numPixelsInX = other.m_numPixelsInX;
    m_numPixelsInY = other.m_numPixelsInY;
    m_availGlyphSlotX = other.m_availGlyphSlotX;
    m_availGlyphSlotY = other.m_availGlyphSlotY;
    m_atlasBytes = std::move(other.m_atlasBytes);
    m_atlasTexture = other.m_atlasTexture;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
GlyphAtlas& GlyphAtlas::operator=(GlyphAtlas&& other)
    {
    if (&other == this)
        return *this;

    m_index = other.m_index;
    m_numGlyphs = other.m_numGlyphs;
    m_numGlyphsInX = other.m_numGlyphsInX;
    m_numGlyphsInY = other.m_numGlyphsInY;
    m_numPixelsInX = other.m_numPixelsInX;
    m_numPixelsInY = other.m_numPixelsInY;
    m_availGlyphSlotX = other.m_availGlyphSlotX;
    m_availGlyphSlotY = other.m_availGlyphSlotY;
    m_atlasBytes = std::move(other.m_atlasBytes);
    m_atlasTexture = other.m_atlasTexture;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool GlyphAtlas::AddGlyph(DeferredGlyph& glyph, GlyphLocation& loc)
    {
    if (m_availGlyphSlotY >= m_numPixelsInY)
        return false; // this atlas is already filled to capacity

    Render::Image* glyphImage = glyph.m_polyface.m_glyphImage;
    ByteStream const& glyphBytes = glyphImage->GetByteStream();

    // copy unique glyph to next available location in the atlas
    for (uint32_t glyphY = 0; glyphY < glyphImage->GetHeight(); glyphY++)
        {
        for (uint32_t glyphX = 0; glyphX < glyphImage->GetWidth(); glyphX++)
            {
            size_t atlasIndex = (m_availGlyphSlotY + glyphY) * m_numPixelsInX * 4 + (m_availGlyphSlotX + glyphX) * 4;
            size_t glyphIndex = glyphY * glyphImage->GetWidth() * 4 + glyphX * 4;
            m_atlasBytes[atlasIndex] = glyphBytes[glyphIndex];
            m_atlasBytes[atlasIndex + 1] = glyphBytes[glyphIndex + 1];
            m_atlasBytes[atlasIndex + 2] = glyphBytes[glyphIndex + 2];
            m_atlasBytes[atlasIndex + 3] = glyphBytes[glyphIndex + 3];
            }
        }

    loc.m_atlasSlotX = m_availGlyphSlotX;
    loc.m_atlasSlotY = m_availGlyphSlotY;

    // calculate next available slot for a glyph
    m_availGlyphSlotX += m_glyphTotalSizeInPixels;
    if (m_availGlyphSlotX >= m_numPixelsInX)
        {
        m_availGlyphSlotX = m_glyphHalfPaddingInPixels;
        m_availGlyphSlotY += m_glyphTotalSizeInPixels;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void GlyphAtlas::GetUVCoords(Render::Image* glyphImage, const GlyphLocation& loc, DPoint2d uvs[2])
    {
    uvs[0].x = loc.m_atlasSlotX / static_cast<double>(m_numPixelsInX);
    uvs[0].y = loc.m_atlasSlotY / static_cast<double>(m_numPixelsInY);
    uvs[1].x = (loc.m_atlasSlotX + glyphImage->GetWidth()) / static_cast<double>(m_numPixelsInX);
    uvs[1].y = (loc.m_atlasSlotY + glyphImage->GetHeight()) / static_cast<double>(m_numPixelsInY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TexturePtr GlyphAtlas::GetTexture(Render::System& renderSystem, DgnDbP db)
    {
    if (!m_atlasTexture.IsNull())
        return m_atlasTexture;

    Render::Image atlasImage(m_numPixelsInX, m_numPixelsInY, std::move(m_atlasBytes), Render::Image::Format::Rgba);

#if defined(DEBUG_DUMP_TEXTURE_ATLAS_DIR)
    writeAtlasToImageFile(atlasImage.GetByteStream().data(), atlasImage.GetWidth(), atlasImage.GetHeight(), atlasImage.GetBytesPerPixel());
#endif

    // create the texture from the atlas image
    Utf8PrintfString name("gta%u", m_index); 
    TextureKey key(name);
    Texture::CreateParams params(key);
    params.m_isGlyph = true;
    m_atlasTexture = renderSystem._CreateTexture(atlasImage, *db, params);

    return m_atlasTexture;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
GlyphAtlasManager::GlyphAtlasManager(uint32_t numGlyphs) : m_numGlyphs(numGlyphs)
    {
    const uint32_t maxGlyphsInAtlas = GlyphAtlas::GetMaxGlyphsInAtlas();
    m_numAtlases = static_cast<uint32_t>(ceil(m_numGlyphs / static_cast<double>(maxGlyphsInAtlas)));
    uint32_t i = 0;
    for (; i < m_numAtlases - 1; i++)
        m_atlases.emplace_back(i, maxGlyphsInAtlas);
    m_atlases.emplace_back(i, m_numGlyphs - maxGlyphsInAtlas * (m_numAtlases - 1));
    m_curAtlasNdx = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void GlyphAtlasManager::AddGlyph(DeferredGlyph& glyph)
    {
    void* key = glyph.GetKey();
    if (m_glyphLocations.end() != m_glyphLocations.find(key))
        return; // glyph already exists in an atlas

    GlyphLocation loc;
    if (!m_atlases[m_curAtlasNdx].AddGlyph(glyph, loc))
        {
        m_curAtlasNdx++;
        m_atlases[m_curAtlasNdx].AddGlyph(glyph, loc);
        }
    loc.m_atlasNdx = m_curAtlasNdx;
    m_glyphLocations.insert(std::pair<void*,GlyphLocation>(key, loc));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
GlyphAtlas& GlyphAtlasManager::GetAtlasAndLocationForGlyph(DeferredGlyph& glyph, GlyphLocation& loc)
    {
    void* key = glyph.GetKey();
    std::map<void*,GlyphLocation>::iterator itr = m_glyphLocations.find(key);
    loc = itr->second;
    return m_atlases[loc.m_atlasNdx];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void GlyphDeferralManager::DeferGlyph(DeferredGlyph& glyph)
    {
    m_deferredGlyphs.push_back(glyph);
    void* key = m_deferredGlyphs.back().GetKey();
    if (m_uniqueGlyphKeys.find(key) == m_uniqueGlyphKeys.end())
        m_uniqueGlyphKeys.insert(key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void GlyphDeferralManager::AddDeferredGlyphsToContext(ViewContext* context)
    {
    uint32_t numGlyphs = static_cast<uint32_t>(m_deferredGlyphs.size());
    if (0 == numGlyphs)
        return;

    GlyphAtlasManager atlasManager(static_cast<uint32_t>(m_uniqueGlyphKeys.size()));
    for (auto& deferredGlyph : m_deferredGlyphs)
        atlasManager.AddGlyph(deferredGlyph);

    // add the polyfaces with appropriate UV coordinates
    for (auto& deferredGlyph : m_deferredGlyphs)
        {
        GlyphLocation loc;
        GlyphAtlas& atlas = atlasManager.GetAtlasAndLocationForGlyph(deferredGlyph, loc);

        // override texture
        TexturePtr tex = atlas.GetTexture(*context->GetRenderSystem(), &context->GetDgnDb());
        deferredGlyph.m_polyface.m_displayParams = deferredGlyph.m_polyface.m_displayParams->CloneForRasterText(*tex);

        // override uvs
        DPoint2d uvs[2];
        atlas.GetUVCoords(deferredGlyph.m_polyface.m_glyphImage, loc, uvs);
        auto& params = deferredGlyph.m_polyface.m_polyface->Param();
        BeAssert(params.size() >= 4);
        for (auto& param : params)
            {
            param.y = param.y < 0.5 ? uvs[0].x : uvs[1].x;
            param.x = param.x < 0.5 ? uvs[0].y : uvs[1].y;
            }

        dynamic_cast<MeshGenerator*>(context)->AddPolyface(deferredGlyph.m_polyface, *deferredGlyph.m_geom, deferredGlyph.m_rangePixels, deferredGlyph.m_isContained, false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void GlyphDeferralManager::AddDeferredGlyphsToBuilderMap(MeshBuilderMap& builderMap, Render::System& renderSystem, DgnDb& db, GeometryOptionsCR options)
    {
    uint32_t numGlyphs = static_cast<uint32_t>(m_deferredGlyphs.size());
    if (0 == numGlyphs)
        return;

    GlyphAtlasManager atlasManager(static_cast<uint32_t>(m_uniqueGlyphKeys.size()));
    for (auto& deferredGlyph : m_deferredGlyphs)
        atlasManager.AddGlyph(deferredGlyph);

    // add the polyfaces with appropriate UV coordinates
    for (auto& deferredGlyph : m_deferredGlyphs)
        {
        GlyphLocation loc;
        GlyphAtlas& atlas = atlasManager.GetAtlasAndLocationForGlyph(deferredGlyph, loc);

        // override texture
        TexturePtr tex = atlas.GetTexture(renderSystem, &db);
        deferredGlyph.m_polyface.m_displayParams = deferredGlyph.m_polyface.m_displayParams->CloneForRasterText(*tex);

        // override uvs
        DPoint2d uvs[2];
        atlas.GetUVCoords(deferredGlyph.m_polyface.m_glyphImage, loc, uvs);
        auto& params = deferredGlyph.m_polyface.m_polyface->Param();
        BeAssert(params.size() >= 4);
        for (auto& param : params)
            {
            param.y = param.y < 0.5 ? uvs[0].x : uvs[1].x;
            param.x = param.x < 0.5 ? uvs[0].y : uvs[1].y;
            }

        DisplayParamsCPtr displayParams = deferredGlyph.m_polyface.m_displayParams;
        bool hasTexture = displayParams.IsValid() && displayParams->IsTextured();

        MeshBuilderMap::Key key(*displayParams, nullptr != deferredGlyph.m_polyface.m_polyface->GetNormalIndexCP(), Mesh::PrimitiveType::Mesh, deferredGlyph.m_polyface.m_isPlanar);
        if (options.WantPreserveOrder())
            key.SetOrder(deferredGlyph.m_order);

        MeshBuilderR meshBuilder = builderMap[key];

        auto edgeOptions = (options.WantEdges() && deferredGlyph.m_polyface.m_displayEdges) ? MeshEdgeCreationOptions::DefaultEdges : MeshEdgeCreationOptions::NoEdges;
        meshBuilder.BeginPolyface(*deferredGlyph.m_polyface.m_polyface, edgeOptions);

        uint32_t fillColor = displayParams->GetFillColor();
        for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*deferredGlyph.m_polyface.m_polyface); visitor->AdvanceToNextFace(); /**/)
            meshBuilder.AddFromPolyfaceVisitor(*visitor, displayParams->GetTextureMapping(), db, deferredGlyph.m_geom->GetFeature(), hasTexture, fillColor, nullptr != deferredGlyph.m_polyface.m_polyface->GetNormalCP(), nullptr);

        meshBuilder.EndPolyface();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddDeferredGlyphMeshes()
    {
    m_glyphDeferralManager.AddDeferredGlyphsToContext(this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddMeshes(GeometryList const& geometries, bool doRangeTest)
    {
    for (auto& geom : geometries)
        {
        if (m_loadContext.WasAborted())
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
    double rangePixels = (m_tile.GetElementRoot().Is3d() ? geomRange.DiagonalDistance() : geomRange.DiagonalDistanceXY()) / m_tolerance;
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
    double rangePixels = (m_tile.GetElementRoot().Is3d() ? geomRange.DiagonalDistance() : geomRange.DiagonalDistanceXY()) / m_tolerance;
    if (rangePixels < s_minRangeBoxSize)
        return;

    // Get the polyfaces and strokes with no transform applied
    PolyfaceList polyfaces = part.GetPolyfaces(m_tolerance, m_options.m_normalMode, nullptr, *this);
    StrokesList strokes = part.GetStrokes(m_tolerance, nullptr, *this);

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
    auto polyfaces = geom.GetPolyfaces(m_tolerance, m_options.m_normalMode, *this);
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
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
static Feature featureFromParams(DgnElementId elemId, DisplayParamsCR params)
    {
    return Feature(elemId, params.GetSubCategoryId(), params.GetClass());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddPolyface(Polyface& tilePolyface, GeometryR geom, double rangePixels, bool isContained, bool doDefer)
    {
    if (nullptr != tilePolyface.m_glyphImage && doDefer)
        {
        // defer processing these until we are finished so we can make texture atlas of glyphs
        DeferredGlyph glyph(tilePolyface, geom, rangePixels, isContained);
        m_glyphDeferralManager.DeferGlyph(glyph);
        return;
        }

    // TFS#817210
    PolyfaceHeaderPtr polyface = tilePolyface.m_polyface.get();
    if (polyface.IsNull() || 0 == polyface->GetPointIndexCount())
        return;

    bool doDecimate = tilePolyface.CanDecimate() && !m_tile.IsLeaf() && !m_tile.HasZoomFactor() && polyface->GetPointCount() > GetDecimatePolyfacePointCount();
    if (doDecimate)
        {
        static constexpr double s_minDecimationRatio = 0.25; // Don't use decimated mesh unless point count reduced by at least this percentage...###TODO revisit - too large?
        BeAssert(0 == polyface->GetEdgeChainCount()); // The decimation does not handle edge chains - but this only occurs for polyfaces which should never have them.

        PolyfaceHeaderPtr decimated;
// #define DECIMATE_BY_EDGE_COLLAPSE
#if defined(DECIMATE_BY_EDGE_COLLAPSE)
        size_t minCollapsed = polyface->GetPointCount() * (1.0 - s_minDecimationRatio);
        size_t numCollapsed = polyface->DecimateByEdgeCollapse(tilePolyface.m_decimationTolerance, 0.0);
        if (numCollapsed >= minCollapsed || true) // ###TODO: In-place decimation prevents this...
            decimated = polyface;
#else
        decimated = polyface->ClusteredVertexDecimate(tilePolyface.m_decimationTolerance, s_minDecimationRatio);
#endif
        if (decimated.IsValid())
            {
            polyface = decimated.get();
            m_didDecimate = true;
            }
        }

#if defined(DISABLE_EDGE_GENERATION)
    auto edgeOptions = MeshEdgeCreationOptions::NoEdges;
#else
    auto edgeOptions = tilePolyface.m_displayEdges ? MeshEdgeCreationOptions::DefaultEdges : MeshEdgeCreationOptions::NoEdges;
#endif

    // NB: You might think we wouldn't need to clip the root tile - but geometry can end up outside the project extents so...

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
        TileRangeClipOutput     clipOutput;

        polyface->ClipToRange(m_tile.GetRange(), clipOutput, false);

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
    DgnDbR              db = m_tile.GetElementRoot().GetDgnDb();
    MeshAuxData         auxData;

    MeshBuilderMap::Key key(displayParams, nullptr != polyface.GetNormalIndexCP(), Mesh::PrimitiveType::Mesh, isPlanar);
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
    auto strokes = geom.GetStrokes(m_tolerance, *this);
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
    if (m_loadContext.WasAborted())
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
MeshList MeshGenerator::GetMeshes(bool isPartialTile)
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

    if (isPartialTile)
        meshes.m_features = m_featureTable;
    else
        meshes.m_features = std::move(m_featureTable);

    return meshes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeshGenerator::ReadFrom(TileTree::StreamBufferR stream, bool& containsCurves, bool& isIncomplete, DgnElementIdSet const& omitElems)
    {
    auto model = m_tile.GetElementRoot().GetModel();
    BeAssert(model.IsValid());
    BeAssert(nullptr != _GetRenderSystem());

    TileTree::IO::DgnTile::Flags tileFlags;

    auto status = TileTree::IO::ReadDgnTile(m_builderMap, stream, *model, *_GetRenderSystem(), tileFlags, omitElems);
    if (TileTree::IO::ReadStatus::Success != status)
        {
        BeAssert(false);
        return false;
        }

    containsCurves = TileTree::IO::DgnTile::Flags::None != (tileFlags & TileTree::IO::DgnTile::Flags::ContainsCurves);
    isIncomplete = TileTree::IO::DgnTile::Flags::None != (tileFlags & TileTree::IO::DgnTile::Flags::Incomplete);

    return true;
    }

BEGIN_ELEMENT_TILETREE_NAMESPACE

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   10/17
//=======================================================================================
struct TileGenerator
{
    enum class Completion
    {
        Full,       // Tile generation completed.
        Partial,    // Tile generation partially completed. Can be resumed.
        Aborted,    // Tile generation aborted (tile load canceled, or tile abandoned).
    };
private:
    IFacetOptionsPtr                            m_facetOptions;
    ElementCollector                            m_elementCollector;
    TileContext                                 m_tileContext;
    MeshGenerator                               m_meshGenerator;
    GeometryList                                m_geometries;
    ElementCollector::Entries::const_iterator   m_elementIter;
    uint32_t                                    m_useCount = 1;

    LoadContextCR GetLoadContext() const { return m_tileContext.GetLoadContext(); }
    TileR GetTile() const { return const_cast<TileR>(m_meshGenerator.GetTile()); } // constructor receives as non-const...
public:
    TileGenerator(DRange3dCR range, RangeIndex::Tree& rangeIndex, double minRangeDiagonalSquared, LoadContextCR loadContext, uint32_t maxElements, TileR tile, IFacetOptionsR facetOptions, TransformCR transformFromDgn, double tolerance, GeometryOptionsCR geomOpts, DgnElementIdSet const* skipElems=nullptr)
    :   m_facetOptions(&facetOptions),
        m_elementCollector(range, rangeIndex, minRangeDiagonalSquared, loadContext, maxElements, skipElems),
        m_tileContext(m_geometries, const_cast<RootR>(tile.GetElementRoot()), range, facetOptions, transformFromDgn, tolerance, loadContext),
        m_meshGenerator(tile, geomOpts, loadContext)
    {
    // ElementCollector has now collected all elements valid for this tile, ordered from largest to smallest
    m_elementIter = m_elementCollector.GetEntries().begin();
    }

    static TileGeneratorUPtr Create(DRange3dCR range, RangeIndex::Tree& rangeIndex, double minRangeDiagonalSquared, LoadContextCR loadContext, uint32_t maxElements, TileR tile, IFacetOptionsR facetOptions, TransformCR transformFromDgn, double tolerance, GeometryOptionsCR geomOpts, TileTree::StreamBufferR stream, DgnElementIdSet const& omitElems, DgnElementIdSet const& tileElems);

    Completion GenerateGeometry(Render::Primitives::GeometryCollection&, LoadContextCR loadContext);
    uint32_t GetAndIncrementUseCount() { return ++m_useCount; }
    bool ReadFrom(TileTree::StreamBufferR buffer, DgnElementIdSet const& omitElems);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeneratorUPtr TileGenerator::Create(DRange3dCR range, RangeIndex::Tree& rangeIndex, double minRangeDiagonalSquared, LoadContextCR loadContext, uint32_t maxElements, TileR tile, IFacetOptionsR facetOptions, TransformCR transformFromDgn, double tolerance, GeometryOptionsCR geomOpts, TileTree::StreamBufferR stream, DgnElementIdSet const& omitElems, DgnElementIdSet const& tileElems)
    {
    TileGeneratorUPtr generator(new TileGenerator(range, rangeIndex, minRangeDiagonalSquared, loadContext, maxElements, tile, facetOptions, transformFromDgn, tolerance, geomOpts, &tileElems));
    if (!generator->ReadFrom(stream, omitElems))
        {
        BeAssert(false);
        return nullptr;
        }

    return generator;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGenerator::ReadFrom(TileTree::StreamBufferR stream, DgnElementIdSet const& omitElems)
    {
    bool containsCurves;
    bool isIncomplete;
    if (!m_meshGenerator.ReadFrom(stream, containsCurves, isIncomplete, omitElems))
        {
        BeAssert(false);
        return false;
        }

    // We have no way of determining that all curved elements were removed from the tile...
    if (containsCurves)
        m_geometries.MarkCurved();

    // Similarly we can't know whether all geometry previously omitted from tile no longer exists or would no longer be omitted...
    if (isIncomplete)
        m_geometries.MarkIncomplete();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::Completion TileGenerator::GenerateGeometry(Render::Primitives::GeometryCollection& output, LoadContextCR loadContext)
    {
    if (loadContext.WasAborted())
        return Completion::Aborted;

    // When we resume tile generation, we have a new Loader, therefore need a new LoadContext...
    m_meshGenerator.SetLoadContext(loadContext);
    m_tileContext.SetLoadContext(loadContext);

    if (m_elementCollector.AnySkipped())
        {
        // We may want to turn this into a leaf node if it contains strictly linear geometry - but we can't do that if any elements were excluded
        m_geometries.MarkIncomplete();
        }

    // Collect geometry from each element, until all elements processed or we run out of time
    bool isPartialTile = false;
    TileR tile = GetTile();
    for (/*m_elementIter*/; m_elementCollector.GetEntries().end() != m_elementIter; ++m_elementIter)
        {
        // Collect geometry from this element
        m_geometries.clear();
        auto const& entry = *m_elementIter;
        m_tileContext.ProcessElement(entry.second, entry.first);
        if (m_tileContext.GetGeometryCount() >= m_elementCollector.GetMaxElements())
            {
            BeAssert(!tile.IsLeaf());
            m_tileContext.TruncateGeometryList(m_elementCollector.GetMaxElements());
            break;
            }

        // Convert this element's geometry to meshes
        for (auto const& geom : m_geometries)
            m_meshGenerator.AddMeshes(*geom, true);

        bool aborted = loadContext.WasAborted();
        isPartialTile = loadContext.WantPartialTiles() && (aborted || loadContext.IsPastCollectionDeadline());
        if (aborted || isPartialTile)
            break;
        }

    if (m_meshGenerator.DidDecimation())
        m_geometries.MarkIncomplete();

    // Don't discard our progress if this is a partial tile
    if (!isPartialTile && loadContext.WasAborted())
        {
        m_geometries.clear();
        return Completion::Aborted;
        }

    // Determine whether or not to subdivide this tile
    bool canSkipSubdivision = tile.GetTolerance() <= s_maxLeafTolerance;
    if (canSkipSubdivision && !isPartialTile && !loadContext.WasAborted() && !tile.IsLeaf() && !tile.HasZoomFactor())
        {
        if (m_geometries.IsComplete() && !m_elementCollector.AnySkipped() && m_elementCollector.GetEntries().size() <= s_minElementsPerTile)
            {
            // We didn't skip any elements and only a small number of elements exist within this tile's range.
            if (!m_geometries.ContainsCurves())
                tile.SetIsLeaf();
            else
                tile.SetZoomFactor(1.0);
            }
        else if (m_elementCollector.GetEntries().size() + m_elementCollector.GetNumSkipped() < s_minElementsPerTile)
            {
            // We skipped some elements within this tile's range, but did not exceed the min elements per tile limitation. Don't subdivide.
            // (TFS#884193 - vast tile ranges containing a handful of teeny-tiny elements)
            tile.SetZoomFactor(1.0);
            }
        }

    if (!isPartialTile)
        m_meshGenerator.AddDeferredGlyphMeshes();

    // Facet all geometry thus far collected to produce meshes.
    Render::Primitives::GeometryCollection collection;
    collection.Meshes() = m_meshGenerator.GetMeshes(isPartialTile);
    if (!isPartialTile)
        {
        tile.SetContentRange(ElementAlignedBox3d(m_meshGenerator.GetContentRange()));
        if (!m_geometries.IsComplete())
            collection.MarkIncomplete();
        }
    else
        {
        collection.MarkIncomplete();
        }

    if (m_geometries.ContainsCurves())
        collection.MarkCurved();

    output = std::move(collection);

    m_geometries.clear(); // NB: Retains curved/complete flags...

    return isPartialTile ? Completion::Partial : Completion::Full;
    }

END_ELEMENT_TILETREE_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Loader::LoadGeometryFromModel()
    {
#if defined (BENTLEYCONFIG_PARASOLID)    
    PSolidThreadUtil::WorkerThreadOuterMark outerMark;
#endif

    SetupForTileRepair();

    auto& tile = GetElementTile();

    LoadContext loadContext(this);
    m_geometry = tile.GenerateGeometry(loadContext);

    return loadContext.WasAborted() ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Loader::SetupForTileRepair()
    {
#if !defined(TEST_TILE_REPAIR)
    if (!IsExpired() && m_omitElemIds.empty())
        return;
#endif

    if (m_tileBytes.empty())
        return;

    auto& tile = GetElementTile();
    auto& root = tile.GetElementRoot();
    auto model = root.GetModel();
    if (model.IsNull() || DgnDbStatus::Success != model->FillRangeIndex())
        {
        BeAssert(false);
        return;
        }

    auto sys = GetRenderSystem();
    BeAssert(nullptr != sys);
    uint32_t maxFeatures = std::min(s_hardMaxFeaturesPerTile, sys->_GetMaxFeaturesPerBatch());

    double minRangeDiagonalSq = s_minRangeBoxSize * tile.GetTolerance();
    IFacetOptionsPtr facetOptions = Geometry::CreateFacetOptions(tile.GetTolerance());
    facetOptions->SetHideSmoothEdgesWhenGeneratingNormals(false);

    Transform transformFromDgn;
    transformFromDgn.InverseOf(root.GetLocationForTileGeneration());

    LoadContext context(this);
    TileGeneratorUPtr generator = TileGenerator::Create(tile.GetDgnRange(), *model->GetRangeIndex(), minRangeDiagonalSq, context, maxFeatures, tile, *facetOptions, transformFromDgn, tile.GetTolerance(), GeometryOptions(), m_tileBytes, m_omitElemIds, m_tileElemIds);
    BeAssert(nullptr != generator);

    m_tileBytes.ResetPos();
    m_omitElemIds.clear();
    m_tileElemIds.clear();

    if (nullptr != generator)
        {
        m_tileBytes.clear();
        tile.SetGenerator(std::move(generator));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Tile::SetGenerator(TileGeneratorUPtr&& generator)
    {
    m_generator = std::move(generator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::~Tile()
    {
    // Defined here for instantiation of destructor of std::unique_ptr<TileGenerator> which is incomplete type in header file.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Render::Primitives::GeometryCollection Tile::GenerateGeometry(LoadContextCR context)
    {
    Render::Primitives::GeometryCollection collection;
    if (nullptr != m_generator.get())
        {
        //THREADLOG.errorv("Refining partial tile (processed %u times)", m_generator->GetAndIncrementUseCount());
        auto status = m_generator->GenerateGeometry(collection, context);
        switch (status)
            {
            case TileGenerator::Completion::Aborted:
                // clear out collection and fall-through...
                collection = Render::Primitives::GeometryCollection();
            case TileGenerator::Completion::Full:
                // no longer need to save generator...fall-through...
                m_generator.reset();
            default:
                return collection;
            }
        }

    auto& root = GetElementRoot();
    auto model = root.GetModel();
    if (model.IsNull() || DgnDbStatus::Success != model->FillRangeIndex())
        return collection;

    uint32_t maxFeatures = s_hardMaxFeaturesPerTile; // Note: Element != Feature - could have multiple features per element due to differing subcategories/classes in GeometryStream
    auto sys = context.GetRenderSystem();
    if (nullptr != sys)
        maxFeatures = std::min(maxFeatures, sys->_GetMaxFeaturesPerBatch());

    double minRangeDiagonalSq = s_minRangeBoxSize * m_tolerance;
    minRangeDiagonalSq *= minRangeDiagonalSq;

    IFacetOptionsPtr facetOptions = Geometry::CreateFacetOptions(m_tolerance);
    facetOptions->SetHideSmoothEdgesWhenGeneratingNormals(false); // We'll do this ourselves when generating meshes - This will turn on sheet edges that should be hidden (Pug.dgn).

    Transform transformFromDgn;
    transformFromDgn.InverseOf(root.GetLocationForTileGeneration());

    // ###TODO: Avoid heap alloc if don't want partial tiles...
    TileGeneratorUPtr generator = std::make_unique<TileGenerator>(GetDgnRange(), *model->GetRangeIndex(), minRangeDiagonalSq, context, maxFeatures, *this, *facetOptions, transformFromDgn, m_tolerance, GeometryOptions());
    auto status = generator->GenerateGeometry(collection, context);
    switch (status)
        {
        case TileGenerator::Completion::Aborted:
            return Render::Primitives::GeometryCollection();
        case TileGenerator::Completion::Partial:
            // Save generator to resume later and fall-through...
            BeAssert(context.WantPartialTiles());
            THREADLOG.warning("Produced partial tile");
            m_generator = std::move(generator);
        default:
            return collection;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d Tile::GetDgnRange() const
    {
    DRange3d range;
    GetElementRoot().GetLocationForTileGeneration().Multiply(range, GetTileRange());
    return range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/17
+---------------+---------------+---------------+---------------+---------------+------*/
Transform Root::GetLocationForTileGeneration() const
    {
    // TFS#783612: Diego's 'Model Alignment' workflow involves:
    // 1. Start a dynamic transaction
    // 2. Apply a uniform transform to all elements in a set of models
    // 3. Apply the same transform to the models' tile trees as the temporary 'display transform' to cause them to render in the new location
    // 4. When finished, cancel the dynamic transaction, which restores the range index; and remove the temporary display transform from the tile trees.
    // In between 2 and 3, we try to generate tiles using the temporarily modified RangeIndex, and our range intersections are all wrong.
    // During tile generation we transform ranges and positions from tile to dgn, and back again - so the net result is as if the
    // RangeIndex had never been modified - we generate the same tiles we normally would.
    auto tf = GetLocation();
    if (m_haveDisplayTransform)
        {
        BeAssert(GetDgnDb().Txns().InDynamicTxn());
        BeAssert(m_ignoreChanges);
        tf = Transform::FromProduct(m_displayTransform, tf);
        }

    return tf;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::SelectParent Tile::SelectTiles(bvector<TileTree::TileCPtr>& selected, TileTree::DrawArgsR args, uint32_t numSkipped) const
    {
    // DgnDb::VerifyClientThread(); ###TODO: Relax this constraint when publishing view attachments to Cesium...

    _ValidateChildren();

    Visibility vis = _GetVisibility(args);
    if (Visibility::OutsideFrustum == vis)
        {
        _UnloadChildren(args.m_purgeOlderThan);
        return SelectParent::No;
        }

    if (Visibility::Visible == vis)
        {
        // This tile is of appropriate resolution to draw.
        // If we need loading or refinement, enqueue
        if (!IsReady() || _IsPartial())
            args.InsertMissing(*this);

        if (_HasGraphics())
            {
            // It can be drawn - select it
            selected.push_back(this);
            if (!_IsPartial())
                _UnloadChildren(args.m_purgeOlderThan);
            }
        else
            {
            // If direct children are drawable, draw them in this tile's place; otherwise draw the parent. Do not load/request the children for this purpose.
            size_t initialSize = selected.size();
            auto children = _GetChildren(false);
            if (nullptr == children)
                return SelectParent::Yes;

            for (auto const& child : *children)
                {
                if (Visibility::OutsideFrustum == child->_GetVisibility(args))
                    continue;

                if (!child->_HasGraphics())
                    {
                    selected.resize(initialSize);
                    return SelectParent::Yes;
                    }

                selected.push_back(child);
                }

            m_childrenLastUsed = BeTimePoint::Now();
            }

        // We're drawing either this tile, or its direct children.
        return SelectParent::No;
        }

    // This tile is too coarse to draw. Try to draw something more appropriate.
    // If it is not ready to draw, we may want to skip loading in favor of loading its descendants.
    // We never skip the root tile(s) to avoid a situation in which we have no tiles to draw - but we can skip them if they're partially loaded.
    bool canSkipThisTile = IsReady() || _IsPartial() || IsParentDisplayable();

    if (canSkipThisTile && IsDisplayable()) // skipping an undisplayable tile (i.e. the root tile) doesn't count toward the maximum
        {
        // Some tiles do not sub-divide - they only facet the same geometry to a higher resolution. We can skip directly to the correct resolution.
        bool isNotReady = !_HasGraphics() && !HasZoomFactor();
        if (isNotReady)
            {
            // We must avoid skipping too many levels, because otherwise we end up generating lots of little tiles for parents which really should have been leaves
            // (We cannot determine if a tile should be a leaf until we actually generate its geometry)
            constexpr uint32_t maxTilesToSkip = 1;
            if (numSkipped >= maxTilesToSkip)
                canSkipThisTile = false;
            else
                ++numSkipped;
            }
        }

    auto children = canSkipThisTile ? _GetChildren(true) : nullptr;
    if (nullptr != children)
        {
        m_childrenLastUsed = BeTimePoint::Now();
        bool allChildrenDrawable = true;
        size_t initialSize = selected.size();

        for (auto const& child : *children)
            {
            if (SelectParent::Yes == static_cast<TileCP>(child.get())->SelectTiles(selected, args, numSkipped))
                {
                // NB: We must continue iterating children so that they can be requested if missing...
                allChildrenDrawable = false;
                }
            }

        if (allChildrenDrawable)
            return SelectParent::No;

        selected.resize(initialSize);
        }


    if (_HasGraphics())
        {
        // This tile might have 'backup' graphics after having been modified. Ask it to load its 'real' graphics.
        // Consider NOT refining partial tile for this purpose? We're awaiting the child tiles.
        if (!IsReady() || _IsPartial())
            args.InsertMissing(*this);

        selected.push_back(this);
        return SelectParent::No;
        }

    // We want to load this tile before considering children.
    args.InsertMissing(*this);
    return IsParentDisplayable() ? SelectParent::Yes : SelectParent::No;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicPtr TileContext::FinishGraphic(GeometryAccumulatorR accum, TileBuilder& builder)
    {
    for (auto& geom : accum.GetGeometries())
        PushGeometry(*geom);

    return m_finishedGraphic; // carries no useful info and is just going to be discarded...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicPtr TileContext::FinishSubGraphic(GeometryAccumulatorR accum, TileSubGraphic& subGf)
    {
    DgnGeometryPartCR input = subGf.GetInput();
    if (!accum.GetGeometries().empty())
        subGf.SetOutput(*GeomPart::Create(input.GetBoundingBox(), accum.GetGeometries()));

    return m_finishedGraphic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileContext::ProcessElement(DgnElementId elemId, double rangeDiagonalSquared)
    {
    try
        {
#ifdef DEBUG_ELEMENT_FILTER
        static DgnElementId             s_debugId((uint64_t) 73634);

        if (s_debugId.IsValid() && s_debugId != elemId)
            return;
#endif

        if (!m_root.GetCachedGeometry(m_geometries, elemId, rangeDiagonalSquared))
            {
            m_curElemId = elemId;
            m_curRangeDiagonalSquared = rangeDiagonalSquared;

            size_t elemGeomIndex = m_geometries.size();

            VisitElement(elemId, false);

            m_root.AddCachedGeometry(m_geometries, elemGeomIndex, elemId, rangeDiagonalSquared);
            }
        }
    catch (...)
        {
        // This shouldn't be necessary - but an uncaught exception will cause the processing to continue forever. (OpenCascade error in LargeHatchPlant.)
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileContext::PushGeometry(GeometryR geom)
    {
    if (!BelowMinRange(geom.GetTileRange()))
        m_geometries.push_back(geom);
    else
        MarkIncomplete();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::ViewFlags TileContext::GetDefaultViewFlags()
    {
    // We need to generate all the geometry - visibility of stuff like text, constructions, etc will be handled in shaders
    // Most default to 'on'
    Render::ViewFlags flags;
    flags.SetShowConstructions(true);
    return flags;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TileContext::_AddSubGraphic(Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, GeometryParamsR geomParams)
    {
    GraphicParams graphicParams;
    _CookGeometryParams(geomParams, graphicParams);
    AddGeomPart(graphic, partId, subToGraphic, geomParams, graphicParams);
    }
                                                                                                                                 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt TileContext::_VisitElement(DgnElementId elementId, bool allowLoad)
    {
    // Never load elements - but do use them if they're already loaded
    DgnElementCPtr el = GetDgnDb().Elements().FindLoadedElement(elementId);
    if (el.IsValid())
        {
        GeometrySourceCP geomElem = el->ToGeometrySource();
        return (nullptr == geomElem) ? ERROR : VisitGeometry(*geomElem);
        }

    // Load only the data we actually need for processing geometry
    // NB: The Step() below as well as each column access requires acquiring the sqlite mutex.
    // Prevent micro-contention by locking the db here
    // Note we do not use a mutex holder because we want to release the mutex before processing the geometry.
    // Also note we use a less-expensive non-recursive mutex for performance.
    GetRoot().GetDbMutex().lock();
    StatusInt status = ERROR;
    auto& stmt = *m_statement;
    stmt.BindInt64(1, static_cast<int64_t>(elementId.GetValueUnchecked()));

    if (BeSQLite::BE_SQLITE_ROW == stmt.Step() && !IsValueNull(1))
        {
        auto geomSrcPtr = m_is3dView ? GeometrySelector3d::ExtractGeometrySource(stmt, GetDgnDb()) : GeometrySelector2d::ExtractGeometrySource(stmt, GetDgnDb());

        stmt.Reset();
        GetRoot().GetDbMutex().unlock();

        if (nullptr != geomSrcPtr)
            status = VisitGeometry(*geomSrcPtr);
        }
    else
        {
        stmt.Reset();
        GetRoot().GetDbMutex().unlock();
        }

    return status;
    }

    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr TileContext::_StrokeGeometry(GeometrySourceCR source, double pixelSize)
    {
    Render::GraphicPtr graphic = source.Draw(*this, pixelSize);
    return WasAborted() ? nullptr : graphic;
    }

//=======================================================================================
// Created specifically for capturing thumbnails. Has a single tile, whose range matches
// that of a ViewContext's frustum and whose graphics are faceted to the tolerance
// exactly appropriate for that frustum.
// @bsistruct                                                   Paul.Connelly   01/18
//=======================================================================================
struct ThumbnailRoot : Root
{
    DEFINE_T_SUPER(Root);
private:
    ThumbnailRoot(GeometricModelR model, TransformCR transform, Render::SystemR system)
        : T_Super(model, transform, system) { }

    void _OnAddToRangeIndex(DRange3dCR, DgnElementId) override { }
    void _OnRemoveFromRangeIndex(DRange3dCR, DgnElementId) override { }
    void _OnUpdateRangeIndex(DRange3dCR, DRange3dCR, DgnElementId) override { }
    void _OnProjectExtentsChanged(AxisAlignedBox3dCR) override { }

    void LoadRootTile(DRange3dCR tileRange, GeometricModelR model);
public:
    virtual ~ThumbnailRoot() { ClearAllTiles(); }

    static RefCountedPtr<ThumbnailRoot> Create(GeometricModelR model, RenderContextR context);
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   01/18
//=======================================================================================
struct ThumbnailTile : Tile
{
    DEFINE_T_SUPER(Tile);
private:
    bool IsCacheable() const override { return false; }

    TileTree::TilePtr _CreateChild(TileTree::OctTree::TileId) const override { BeAssert(false); return nullptr; }
    void _Invalidate() override { }
    bool _IsInvalidated(TileTree::DirtyRangesCR) const override { return false; }
    void _UpdateRange(DRange3dCR, DRange3dCR) override { }

    bool _HasChildren() const override { return false; }
    ChildTiles const* _GetChildren(bool) const override { return nullptr; }
    void _ValidateChildren() const override { }
    Utf8String _GetTileCacheKey() const override { return "NotCacheable!"; }

    SelectParent _SelectTiles(bvector<TileTree::TileCPtr>& selected, TileTree::DrawArgsR args) const override;
public:
    ThumbnailTile(DRange3dCR range, ThumbnailRoot& root, double minToleranceRatio) : T_Super(root, TileTree::OctTree::TileId::RootId(), range, minToleranceRatio)
        {
        //
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::SelectParent ThumbnailTile::_SelectTiles(bvector<TileTree::TileCPtr>& selected, TileTree::DrawArgsR args) const
    {
    BeAssert(nullptr == GetParent());
    BeAssert(selected.empty());

    selected.push_back(this);

    if (!IsReady())
        args.InsertMissing(*this);

    return SelectParent::No;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ThumbnailRoot> ThumbnailRoot::Create(GeometricModelR model, RenderContextR context)
    {
    BeAssert(DrawPurpose::CaptureThumbnail == context.GetDrawPurpose());
    BeAssert(nullptr != context.GetRenderSystem());

    if (nullptr == context.GetRenderSystem() || DgnDbStatus::Success != model.FillRangeIndex())
        return nullptr;

#if defined (BENTLEYCONFIG_PARASOLID)
    PSolidKernelManager::StartSession();

    if (s_psolidMainThreadMark.IsNull())
        s_psolidMainThreadMark = new PSolidThreadUtil::MainThreadMark();
#endif

    DRange3d frustumRange = context.GetViewportR().GetFrustum().ToRange();
    if (model.Is3dModel())
        frustumRange = scaleSpatialRange(frustumRange);

    DPoint3d centroid = DPoint3d::FromInterpolate(frustumRange.low, 0.5, frustumRange.high);
    Transform transform = Transform::From(centroid);

    auto root = new ThumbnailRoot(model, transform, *context.GetRenderSystem());

    Transform rangeTransform;
    rangeTransform.InverseOf(transform);
    DRange3d tileRange;
    rangeTransform.Multiply(tileRange, frustumRange);

    // This tile's size on screen in pixels matches the size of the view rect...
    BSIRect viewRect = context.GetViewportR().GetViewRect();
    double width = viewRect.Width();
    double height = viewRect.Height();
    double minToleranceRatio = sqrt(width*width + height*height) * 2.0;

    root->m_rootTile = new ThumbnailTile(tileRange, *root, minToleranceRatio);

    return root;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
RootPtr Root::Create(GeometricModelR model, RenderContextR context)
    {
    return ThumbnailRoot::Create(model, context).get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RealityData::CachePtr TileCache::Create(DgnDbCR db)
    {
    RealityData::CachePtr cache(new TileCache(1024*1024*1024));
    BeFileName cacheName = T_HOST.GetTileAdmin()._GetElementCacheFileName(db);
    if (SUCCESS != cache->OpenAndPrepare(cacheName))
        cache = nullptr;

    return cache;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP TileCache::GetCurrentVersion()
    {
    // Increment this when the binary tile format changes...
    // Version history:
    //  1: Added version to cache db property table; fixed various pre-existing serialization bugs
    //  2: Removed rangeCenter from polylines
    //  3: Fixed ordering of DisplayParams::operator<() to use stable IDs of materials and textures rather than addresses,
    //     to ensure same order when deserializing from cache data back into MeshBuilderMap.
    //  4: Prevent unused vertices from ending up in meshes due to degenerate triangles (detect degenerates *before* adding vertices)
    //  5: Ensure curved, stroked curve vectors are treated as curved
    //  6: Treat transparency < 15 as opaque (affects mesh batching)
    //  7: Fix out-of-control subdivision in specific cases (TFS#884193, TFS#886685)
    //  8: Fix issue in which MeshGenerator::AddPolyface() decimates a polyface in a tile which later becomes a leaf tile (TFS#889847)
    //  9: We were failing to align most chunks of the binary data. In JavaScript this produces exceptions when trying to read unaligned data.
    //  10: Change tile cache key such that if tile has no 'zoom factor', it is written as 0, not 1.
    //  11: Added width + height to textures.
    //  12: Fixed display priority range in 2d models
    //  13: Texture atlas for raster text
    return "13";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileCache::WriteCurrentVersion() const
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
BentleyStatus TileCache::_Initialize() const
    {
    // We've created a brand-new cache Db. Write the current binary format version to its property table.
    return WriteCurrentVersion() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileCache::_ValidateData() const
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
* @bsimethod                                                    Paul.Connelly   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Root::_ToJson(Json::Value& json) const
    {
    json["id"] = GetModelId().ToHexStr();
    json["maxTilesToSkip"] = 1;
    JsonUtils::TransformToJson(json["location"], GetLocation());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Tile::_ToJson(Json::Value& json) const
    {
    json["id"]["treeId"] = GetElementRoot().GetModelId().ToHexStr();
    json["id"]["tileId"] = GetIdString();
    json["maximumSize"] = _GetMaximumSize();

    JsonUtils::DRange3dToJson(json["range"], GetRange());
    if (HasContentRange())
        JsonUtils::DRange3dToJson(json["contentRange"], _GetContentRange());

    if (HasZoomFactor())
        json["zoomFactor"] = GetZoomFactor();

    if (nullptr != GetParent())
        json["parentId"] = GetElementParent()->GetIdString();
    
    auto children = _GetChildren(true);
    json["childIds"] = Json::arrayValue;
    if (nullptr != children)
        {
        for (auto const& child : *children)
            json["childIds"].append(static_cast<TileR>(*child).GetIdString());
        }

    ByteStream geometry = GetRoot().GetTileDataFromCache(_GetTileCacheKey());
    if (!geometry.empty())
        {
        Utf8String encodedGeometry;
        Base64Utilities::Encode(encodedGeometry, geometry.GetData(), geometry.size());
        json["geometry"] = encodedGeometry;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Tile::GetIdString() const
    {                                                                                                                                                                       
    return Utf8PrintfString("%u/%u/%u/%u:%f", m_id.m_level, m_id.m_i, m_id.m_j, m_id.m_k, m_zoomFactor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::TilePtr Root::_FindTileById(Utf8CP strId)
    {
    if (GetRootTile().IsNull())
        return nullptr;
    
    TileTree::OctTree::TileId id;
    double zoomFactor;
    if (nullptr == strId || 5 != BE_STRING_UTILITIES_UTF8_SSCANF(strId, "%" SCNu8 "/%u/%u/%u:%lf", &id.m_level, &id.m_i, &id.m_j, &id.m_k, &zoomFactor))
        {
        BeAssert(false && "Invalid tile id string");
        return nullptr;
        }

    return static_cast<TileR>(*GetRootTile()).FindTile(id, zoomFactor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TilePtr Tile::FindTile(TileTree::OctTree::TileId id, double zoomFactor)
    {
    if (id == GetTileId())
        return FindTile(zoomFactor);

    auto children = _GetChildren(true);
    if (nullptr == children)
        return nullptr;

    if (id.m_level <= m_id.m_level)
        {
        BeAssert(false);
        return nullptr;
        }

    uint32_t    shift = id.m_level - m_id.m_level - 1;

    auto imod = id.m_i >> shift, jmod = id.m_j >> shift, kmod = id.m_k >> shift;
    for (auto const& child : *children)
        {
        auto elemChild = static_cast<TileP>(child.get());
        auto childId = elemChild->GetTileId();
        if (childId.m_i == imod && childId.m_j== jmod && childId.m_k == kmod)
            return elemChild->FindTile(id, zoomFactor);
        }

    BeAssert(false && "missing child tile");
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TilePtr Tile::FindTile(double zoomFactor)
    {
    if (zoomFactor == m_zoomFactor)
        return this;

    BeAssert(zoomFactor > 0.0 && HasZoomFactor());
    auto children = _GetChildren(true);
    if (nullptr == children || 1 != children->size())
        {
        BeAssert(false);
        return nullptr;
        }

    return static_cast<TileR>(**children->begin()).FindTile(zoomFactor);
    }


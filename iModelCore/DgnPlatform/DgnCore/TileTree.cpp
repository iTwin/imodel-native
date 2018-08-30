/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/TileTree.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <numeric>
#include <DgnPlatform/RangeIndex.h>
#include <DgnPlatform/TileReader.h>
#include <GeomJsonWireFormat/JsonUtils.h>
#include <numeric>
#include <inttypes.h>
#if defined (BENTLEYCONFIG_PARASOLID) 
#include <DgnPlatform/DgnBRep/PSolidUtil.h>
#endif

USING_NAMESPACE_TILETREE
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES

using FBox3d = RangeIndex::FBox;

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

// Uncomment to disable edge generation
// #define DISABLE_EDGE_GENERATION

BEGIN_UNNAMED_NAMESPACE

struct TileContext;

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
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Loader : TileLoader
{
    DEFINE_T_SUPER(TileLoader);

private:
    Render::Primitives::GeometryCollection  m_geometry;

    BentleyStatus _GetFromSource() override;
    BentleyStatus _LoadTile() override;

    BentleyStatus LoadGeometryFromModel();
    BentleyStatus DoGetFromSource();

    Loader(TileR tile, TileLoadStateSPtr loads) : T_Super(tile, loads) { }
public:
    static TileLoaderPtr Create(TileR tile, TileLoadStateSPtr loads) { return new Loader(tile, loads); }
};

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
    Render::SystemP _GetRenderSystem() const override { return &m_loadContext.GetRenderSystem(); }
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
    System& GetRenderSystemR() const { return m_loadContext.GetRenderSystem(); }
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
        tileTransform.InverseOf(m_context.GetRoot().GetLocation());
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
    double              maxDimension = (nullptr == (lineStyle = lsSymb.GetILineStyle())) ? 0.0 : std::max(lineStyle->GetMaxWidth(), lineStyle->GetLength());
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
#if defined (BENTLEYCONFIG_PARASOLID)
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
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
static GeomPartPtr generateGeomPart(DgnGeometryPartId partId, Render::GeometryParamsR geomParams, TileContext& context)
    {
    GeomPartPtr part;
    DgnGeometryPartCPtr geomPart = context.GetDgnDb().Elements().Get<DgnGeometryPart>(partId);
    if (geomPart.IsValid())
        part = context.GenerateGeomPart(*geomPart, geomParams);

    return part;
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

    SystemP _GetRenderSystem() const override { return &m_loadContext.GetRenderSystem(); }
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

    // Return a list of all meshes currently in the builder map
    MeshList GetMeshes();
    // Return a tight bounding volume
    DRange3dCR GetContentRange() const { return m_contentRange; }
    DRange3dCR GetTileRange() const { return m_builderMap.GetRange(); }
    TileCR GetTile() const { return m_tile; }
    void SetLoadContext(LoadContextCR context) { m_loadContext = context; }

    bool ReadFrom(StreamBufferR, bool& containsCurves, bool& isIncomplete, DgnElementIdSet const& omitElems);

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
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
static Feature featureFromParams(DgnElementId elemId, DisplayParamsCR params)
    {
    return Feature(elemId, params.GetSubCategoryId(), params.GetClass());
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   10/17
//=======================================================================================
struct TileGenerator
{
    enum class Completion
    {
        Full,       // Tile generation completed.
        Aborted,    // Tile generation aborted (tile load canceled, or tile abandoned).
    };
private:
    IFacetOptionsPtr                            m_facetOptions;
    ElementCollector                            m_elementCollector;
    TileContext                                 m_tileContext;
    MeshGenerator                               m_meshGenerator;
    GeometryList                                m_geometries;
    ElementCollector::Entries::const_iterator   m_elementIter;

    LoadContextCR GetLoadContext() const { return m_tileContext.GetLoadContext(); }
    TileR GetTile() const { return const_cast<TileR>(m_meshGenerator.GetTile()); } // constructor receives as non-const...
public:
    TileGenerator(DRange3dCR range, RangeIndex::Tree& rangeIndex, double minRangeDiagonalSquared, LoadContextCR loadContext, uint32_t maxElements, TileR tile, IFacetOptionsR facetOptions, TransformCR transformFromDgn, double tolerance, GeometryOptionsCR geomOpts, DgnElementIdSet const* skipElems=nullptr)
    :   m_facetOptions(&facetOptions),
        m_elementCollector(range, rangeIndex, minRangeDiagonalSquared, loadContext, maxElements, skipElems),
        m_tileContext(m_geometries, const_cast<RootR>(tile.GetRoot()), range, facetOptions, transformFromDgn, tolerance, loadContext),
        m_meshGenerator(tile, geomOpts, loadContext)
    {
    // ElementCollector has now collected all elements valid for this tile, ordered from largest to smallest
    m_elementIter = m_elementCollector.GetEntries().begin();
    }

    Completion GenerateGeometry(Render::Primitives::GeometryCollection&, LoadContextCR loadContext);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Render::Primitives::GeometryCollection generateTileGeometry(TileR tile, LoadContextCR context)
    {
    Render::Primitives::GeometryCollection collection;
    auto& root = tile.GetRoot();
    auto model = root.GetModel();
    if (model.IsNull() || DgnDbStatus::Success != model->FillRangeIndex())
        return collection;

    uint32_t maxFeatures = s_hardMaxFeaturesPerTile; // Note: Element != Feature - could have multiple features per element due to differing subcategories/classes in GeometryStream
    auto& sys = context.GetRenderSystem();
    maxFeatures = std::min(maxFeatures, sys._GetMaxFeaturesPerBatch());

    auto tolerance = tile.GetTolerance();
    double minRangeDiagonalSq = s_minRangeBoxSize * tolerance;
    minRangeDiagonalSq *= minRangeDiagonalSq;

    IFacetOptionsPtr facetOptions = Geometry::CreateFacetOptions(tolerance);
    facetOptions->SetHideSmoothEdgesWhenGeneratingNormals(false); // We'll do this ourselves when generating meshes - This will turn on sheet edges that should be hidden (Pug.dgn).

    Transform transformFromDgn;
    transformFromDgn.InverseOf(root.GetLocation());

    // ###TODO: Avoid heap alloc if don't want partial tiles...
    // ###TODO_IMODELCORE: Removed support for partial tiles - remove heap alloc.
    TileGenerator generator(tile.GetDgnRange(), *model->GetRangeIndex(), minRangeDiagonalSq, context, maxFeatures, tile, *facetOptions, transformFromDgn, tolerance, GeometryOptions());
    auto status = generator.GenerateGeometry(collection, context);
    switch (status)
        {
        case TileGenerator::Completion::Aborted:
            return Render::Primitives::GeometryCollection();
        default:
            return collection;
        }
    }

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileLoader::TileLoader(TileR tile, TileLoadStateSPtr loads) : m_tile(&tile), m_loads(loads), m_cacheKey(tile.GetRoot().ConstructTileResource(tile)),
    m_createTime(tile.GetRoot().GetModel()->GetLastElementModifiedTime()), m_cacheCreateTime(m_createTime)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileLoader::IsExpired(uint64_t createTimeMillis)
    {
    m_cacheCreateTime = createTimeMillis;
    auto& tile = *m_tile;
    uint64_t lastModMillis = tile.GetRoot().GetModel()->GetLastElementModifiedTime();
    return createTimeMillis < static_cast<uint64_t>(lastModMillis);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileLoader::IsValidData()
    {
    BeAssert(!m_tileBytes.empty());
    Dgn::Tile::IO::DgnTileReader reader(m_tileBytes, *m_tile->GetRoot().GetModel(), GetRenderSystem());
    return reader.VerifyFeatureTable();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016                                        
//----------------------------------------------------------------------------------------
BentleyStatus TileLoader::LoadTile()
    {
    // During the read we may have abandoned the tile. Do not waste time loading it.
    if (IsCanceledOrAbandoned())
        return ERROR;

    BeAssert(m_tile->IsQueued());
    return _LoadTile();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
BentleyStatus TileLoader::Perform()
    {
    m_tile->SetIsQueued(); // mark as queued so we don't request it again.

    TileLoaderPtr me(this);
    auto loadFlag = std::make_shared<LoadFlag>(m_loads);  // Keep track of running requests so we can exit gracefully.

    auto status = ReadFromDb();
    if (SUCCESS != status)
        status = me->IsCanceledOrAbandoned() ? ERROR : me->_GetFromSource();

    auto& tile = *me->m_tile;
    if (SUCCESS != status || SUCCESS != me->LoadTile())
        {
        if (me->m_loads != nullptr && me->m_loads->IsCanceled())
            tile.SetNotLoaded();     // Mark it as not loaded so we can retry again.
        else
            tile.SetNotFound();

        return ERROR;
        }

    tile.SetIsReady();   // OK, we're all done loading and the other thread may now use this data. Set the "ready" flag.

    // On a successful load, potentially store the tile in the cache.   
    me->SaveToDb();

    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  12/2016
//----------------------------------------------------------------------------------------
BentleyStatus TileLoader::ReadFromDb()
    {
    auto cache = m_tile->GetRoot().GetCache();
    if (!cache.IsValid())
        return ERROR;

    TileLoaderPtr me(this);
    return me->IsCanceledOrAbandoned() ? ERROR : me->DoReadFromDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileLoader::DropFromDb(RealityData::CacheR cache)
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
* Attempt to load a tile from the local cache.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileLoader::DoReadFromDb()
    {
    auto cache = m_tile->GetRoot().GetCache();
    if (!cache.IsValid())
        return ERROR;

    if (true)
        {
        RealityData::Cache::AccessLock lock(*cache); // block writes to cache Db while we're reading

        enum Column : int {Data,DataSize,Metadata,Created,Rowid};
        CachedStatementPtr stmt;
        constexpr Utf8CP selectSql = "SELECT Data,DataSize,Metadata,Created," TABLE_NAME_TileTree ".ROWID as TileRowId"
            " FROM " JOIN_TileTreeTables " WHERE " COLUMN_TileTree_TileId "=?";

        if (BE_SQLITE_OK != cache->GetDb().GetCachedStatement(stmt, selectSql))
            {
            BeAssert(false);
            return ERROR;
            }

        stmt->ClearBindings();
        stmt->BindText(1, m_cacheKey, Statement::MakeCopy::No);
        if (BE_SQLITE_ROW != stmt->Step())
            return ERROR;

        uint64_t rowId = stmt->GetValueInt64(Column::Rowid);
        uint64_t createTime = stmt->GetValueInt64(Column::Created);
        if (IsExpired(createTime))
            {
            DropFromDb(*cache);
            return ERROR;
            }

        if (0 == stmt->GetValueInt64(Column::DataSize))
            {
            m_tileBytes.clear();
            }
        else
            {
            if (ZIP_SUCCESS != m_snappyFrom.Init(cache->GetDb(), TABLE_NAME_TileTree, "Data", rowId))
                {
                BeAssert(false);
                return ERROR;
                }

            CacheBlobHeader     header(m_snappyFrom);
            uint32_t            sizeRead;

            if ((CacheBlobHeader::DB_Signature06 != header.m_signature) || 0 == header.m_size)
                {
                BeAssert(false);
                return ERROR;
                }

            m_tileBytes.Resize(header.m_size);
            m_snappyFrom.ReadAndFinish(m_tileBytes.GetDataP(), header.m_size, sizeRead);

            if (sizeRead != header.m_size)
                {
                BeAssert(false);
                return ERROR;
                }

            m_tileBytes.SetPos(0);
            if (!IsValidData())
                {
                m_tileBytes.clear();
                DropFromDb(*cache);
                return ERROR;
                }
            }

        m_tileBytes.SetPos(0);
        Json::Value json;
        Json::Reader::Parse(stmt->GetValueText(Column::Metadata), json);
        m_tileMetadata.FromJson(json);
        
        m_saveToCache = false;  // We just load the data from cache don't save it and update timestamp only.

        if (BE_SQLITE_OK == cache->GetDb().GetCachedStatement(stmt, "UPDATE " TABLE_NAME_TileTreeCreateTime " SET Created=? WHERE TileId=?"))
            {
            stmt->BindInt64(1, GetCreateTime());
            stmt->BindText(2, m_cacheKey, Statement::MakeCopy::No);
            if (BE_SQLITE_DONE != stmt->Step())
                {
                BeAssert(false);
                }
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
ByteStream Root::GetTileDataFromCache(Utf8StringCR cacheKey) const
    {
    ByteStream bytes;
    auto cache = GetCache();
    if (cache.IsNull())
        return bytes;

    if (true)
        {
        RealityData::Cache::AccessLock lock(*cache);

        enum Column : int {Data,DataSize,Rowid};

        CachedStatementPtr stmt;
        constexpr Utf8CP selectSql = "SELECT Data,DataSize," TABLE_NAME_TileTree ".ROWID as TileRowId FROM " TABLE_NAME_TileTree " WHERE TileId=?";

        if (BE_SQLITE_OK != cache->GetDb().GetCachedStatement(stmt, selectSql))
            {
            BeAssert(false);
            return bytes;
            }

        stmt->ClearBindings();
        stmt->BindText(1, cacheKey, Statement::MakeCopy::No);
        if (BE_SQLITE_ROW != stmt->Step())
            return bytes;

        uint64_t rowId = stmt->GetValueInt64(Column::Rowid);
        if (0 == stmt->GetValueInt64(Column::DataSize))
            return bytes;

        BeSQLite::SnappyFromBlob snappyFrom;
        if (ZIP_SUCCESS != snappyFrom.Init(cache->GetDb(), TABLE_NAME_TileTree, "Data", rowId))
            return bytes;

        CacheBlobHeader header(snappyFrom);
        uint32_t sizeRead;
        if ((CacheBlobHeader::DB_Signature06 != header.m_signature) || 0 == header.m_size)
            return bytes;

        bytes.Resize(header.m_size);
        snappyFrom.ReadAndFinish(bytes.GetDataP(), header.m_size, sizeRead);
        if (sizeRead != header.m_size)
            bytes.Clear();

        return bytes;
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  12/2016
//----------------------------------------------------------------------------------------
BentleyStatus TileLoader::SaveToDb()
    {
    if (!m_saveToCache)
        return SUCCESS;

    auto cache = m_tile->GetRoot().GetCache();
    if (!cache.IsValid())
        return ERROR;

    TileLoaderPtr me(this);
    return me->DoSaveToDb();
    }

/*---------------------------------------------------------------------------------**//**
* Save the data for a tile into the tile cache. Note that this is also called for the non-tile files.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileLoader::DoSaveToDb()
    {
    if (!m_saveToCache)
        return SUCCESS;

    auto cache = m_tile->GetRoot().GetCache();
    if (!cache.IsValid())
        return ERROR; 

    BeAssert(!m_cacheKey.empty());

    RealityData::Cache::AccessLock lock(*cache);

    // "INSERT OR REPLACE" so we can update old data that we failed to load.
    CachedStatementPtr stmt;
    auto rc = cache->GetDb().GetCachedStatement(stmt, "INSERT OR REPLACE INTO " TABLE_NAME_TileTree " (TileId,Data,DataSize,Metadata) VALUES (?,?,?,?)");

    BeAssert(rc == BE_SQLITE_OK);
    BeAssert(stmt.IsValid());

    stmt->ClearBindings();
    stmt->BindText(1, m_cacheKey, Statement::MakeCopy::No);
    if (m_tileBytes.empty())
        {
        stmt->BindZeroBlob(2, 0);
        stmt->BindInt64(3, 0);
        }
    else
        {
        m_snappyTo.Init();
        CacheBlobHeader header(m_tileBytes.GetSize());
        m_snappyTo.Write((Byte const*) &header, sizeof(header));
        m_snappyTo.Write(m_tileBytes.GetData(), (int) m_tileBytes.GetSize());
        uint32_t zipSize = m_snappyTo.GetCompressedSize();
        stmt->BindZeroBlob(2, zipSize); 
        stmt->BindInt64(3, (int64_t) zipSize);
        }

    Utf8String metadata = Json::FastWriter::ToString(m_tileMetadata.ToJson());
    stmt->BindText(4, metadata, Statement::MakeCopy::No);

    rc = stmt->Step();
    if (BE_SQLITE_DONE != rc)
        {
        BeAssert(false);
        return ERROR;
        }

    // Compress and save the data
    rc = cache->GetDb().GetCachedStatement(stmt, "SELECT ROWID FROM " TABLE_NAME_TileTree " WHERE TileId=?");
    BeAssert(BE_SQLITE_OK == rc && stmt.IsValid());

    stmt->BindText(1, m_cacheKey, Statement::MakeCopy::No);
    if (BE_SQLITE_ROW != stmt->Step())
        {
        BeAssert(false);
        return ERROR;
        }

    uint64_t rowId = stmt->GetValueInt64(0);

    if (!m_tileBytes.empty())
        {
        StatusInt status = m_snappyTo.SaveToRow( cache->GetDb(), TABLE_NAME_TileTree, "Data", rowId);
        if (SUCCESS != status)
            {
            BeAssert(false);
            return ERROR;
            }
        }

    // Write the tile creation time into separate table so that when we update it on next use of this tile, sqlite doesn't have to copy the potentially-huge data column
    rc = cache->GetDb().GetCachedStatement(stmt, "INSERT OR REPLACE INTO " TABLE_NAME_TileTreeCreateTime " (TileId,Created) VALUES (?,?)");
    BeAssert(BE_SQLITE_OK == rc && stmt.IsValid());

    stmt->BindText(1, m_cacheKey, Statement::MakeCopy::No);
    stmt->BindInt64(2, GetCreateTime());
    if (BE_SQLITE_DONE != stmt->Step())
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Create the table to hold entries in this TileCache
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileCache::_Prepare() const 
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
BentleyStatus TileCache::_Cleanup() const 
    {
    AccessLock lock(const_cast<TileCache&>(*this));

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
RealityData::CachePtr TileCache::Create(DgnDbCR db)
    {
    RealityData::CachePtr cache(new TileCache(1024*1024*1024));
    BeFileName cacheName = db.GetFileName();
    cacheName.AppendExtension(L"Tiles");
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
    // We changed the cache db schema shortly after creating imodel02 branch - so version history restarts at same time.
    // 0: Initial version following db schema change
    // 1: Do not set 'is leaf' if have size multiplier ('zoom factor'); add flag and value for size multiplier
    return "0";
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
bool TileCache::ValidateData() const
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
* @bsimethod                                    Keith.Bentley                   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::ClearAllTiles()
    {
    if (!m_rootTile.IsValid())
        return;

    m_rootTile->SetAbandoned();
    WaitForAllLoads();

    m_rootTile = nullptr;
    m_cache = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Root::RequestTile(TileR tile)
    {
    if (!tile.IsNotLoaded())
        return SUCCESS;

    auto loads = std::make_shared<TileLoadState>(tile);

    TileLoaderPtr loader = tile.CreateTileLoader(loads);
    if (!loader.IsValid())
        return ERROR;   
    
    return loader->Perform();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
Root::Root(GeometricModelCR model, TransformCR location, Render::SystemR system, bool asClassifier)
    : m_db(model.GetDgnDb()), m_location(location), m_renderSystem(system), m_modelId(model.GetModelId()), m_is3d(model.Is3d()), m_cache(model.GetDgnDb().ElementTileCache())
    {
    if (asClassifier)
        m_classificationPreprocessor = new ClassificationPreprocessor(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::StartTileLoad(TileLoadStateSPtr state) const
    {
    BeAssert(nullptr != state);
    BeMutexHolder lock(m_cv.GetMutex());
    BeAssert(m_activeLoads.end() == m_activeLoads.find(state));
    m_activeLoads.insert(state);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::DoneTileLoad(TileLoadStateSPtr state) const
    {
    BeAssert(nullptr != state);
    BeMutexHolder lock(m_cv.GetMutex());
    BeAssert(m_activeLoads.end() != m_activeLoads.find(state));
    m_activeLoads.erase(state);
    m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Root::ToJson(Json::Value& json) const
    {
    json["id"] = (Utf8String(IsClassifier() ? "Classifier_" : "") + GetModelId().ToHexStr()).c_str();
    json["maxTilesToSkip"] = 1;
    JsonUtils::TransformToJson(json["location"], GetLocation());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::Tile::Tile(RootR root, TileId id, TileCP parent, bool isLeaf)
    : m_root(root), m_parent(parent), m_depth(nullptr == parent ? 0 : parent->GetDepth() + 1), m_loadStatus(LoadStatus::NotLoaded), m_id(id)
    {
    SetIsLeaf(isLeaf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::Tile::Tile(Root& octRoot, TileId id, Tile const* parent, DRange3dCP range, bool displayable)
    : TileTree::Tile(octRoot, id, parent, false)
    {
    if (nullptr != parent)
        m_range = ElementAlignedBox3d(parent->ComputeChildRange(*this, octRoot.Is2d()));
    else
        m_range.Extend(*range);

    InitTolerance(s_minToleranceRatio);
    }

/*---------------------------------------------------------------------------------**//**
* NB: Constructor used by ThumbnailTile...
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::Tile::Tile(Root& root, TileId id, DRange3dCR range, double minToleranceRatio)
    : Tile(root, id, nullptr, true)
    {
    m_range.Extend(range);
    InitTolerance(minToleranceRatio, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::Tile::Tile(Tile const& parent) : Tile(const_cast<Root&>(parent.GetRoot()), parent.GetTileId(), &parent, false)
    {
    m_range.Extend(parent.GetRange());

    BeAssert(parent.HasZoomFactor());
    SetZoomFactor(parent.GetZoomFactor() * 2.0);

    InitTolerance(s_minToleranceRatio);
    }

/*---------------------------------------------------------------------------------**//**
* This method gets called on the (valid) children of nodes as they are unloaded. Its purpose is to notify the loading
* threads that these nodes are no longer referenced and we shouldn't waste time loading them.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileTree::Tile::SetAbandoned() const
    {
    for (auto const& child : m_children)
        child->SetAbandoned();

    // this is actually a race condition, but it doesn't matter. If the loading thread misses the abandoned flag, the only consequence is we waste a little time.
    m_loadStatus.store(LoadStatus::Abandoned);
    }

/*---------------------------------------------------------------------------------**//**
* Unload all of the children of this node. Must be called from client thread. Usually this will cause the nodes to become unreferenced
* and therefore deleted. Note that sometimes we can unload a child that is still in the download queue. In that case, it will remain alive until
* it arrives. Set its "abandoned" flag to tell the download thread it can skip it (it will get deleted when the download thread releases its reference to it.)
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileTree::Tile::UnloadChildren(BeTimePoint olderThan) const
    {
    if (m_children.empty())
        return;

    // ###TODO_IMODELCORE: Any use for the timestamp on backend? if (m_childrenLastUsed > olderThan) // have we used this node's children recently?
    if (false)
        {
        // yes, this node has been used recently. We're going to keep it, but potentially unload its grandchildren
        for (auto const& child : m_children)
            child->UnloadChildren(olderThan);

        return;
        }

    for (auto const& child : m_children)
        child->SetAbandoned();

    m_children.clear();
    }

/*---------------------------------------------------------------------------------**//**
* ensure that this Tile's range includes its child's range.
* @bsimethod                                    Keith.Bentley                   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileTree::Tile::ExtendRange(DRange3dCR childRange) const
    {
    if (childRange.IsContained(m_range))
        return;

    m_range.Extend(childRange);

    if (nullptr != m_parent)
        m_parent->ExtendRange(childRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileTree::Tile::IsEmpty() const
    {
    // NB: A parent tile may be empty because the elements contained within it are all too small to contribute geometry -
    // children may not be empty.
    return IsReady() && !IsDisplayable() && !HasChildren();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementAlignedBox3d TileTree::Tile::ComputeRange() const
    {
    if (m_range.IsValid())
        return m_range;

    auto const* children = GetChildren(true); // only returns fully loaded children
    if (nullptr != children)
        {
        for (auto const& child : *children)
            m_range.UnionOf(m_range, child->ComputeRange()); // this updates the range of the top level node
        }

    return m_range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileLoadState::~TileLoadState()
    {
    if (m_canceled.load())
        {
#if defined(DEBUG_TILE_CANCEL)
        THREADLOG.errorv("Tile load canceled", m_canceled.load());
#endif
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::Tile::ChildTiles const* TileTree::Tile::GetChildren(bool load) const
    {
    if (IsLeaf())
        return nullptr;

    if (load && m_children.empty())
        {
        if (HasZoomFactor())
            {
            // Create a single child containing same geometry in same range, faceted to a higher resolution.
            m_children.push_back(CreateWithZoomFactor(*this));
            }
        else
            {
            for (int i = 0; i < 2; i++)
                {
                for (int j = 0; j < 2; j++)
                    {
                    for (int k = 0; k < 2; k++)
                        {
                        auto child = CreateChild(m_id.CreateChildId(i, j, k));
                        if (child.IsValid())
                            m_children.push_back(child);
                        }
                    }
                }
            }
        }

    return m_children.empty() ? nullptr : &m_children;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TileTree::Tile::GetTileCacheKey() const
    {
    return Utf8String(GetRoot().IsClassifier() ? "Classifier_" : "") + GetRoot().GetModelId().ToString() + GetIdString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
double TileTree::Tile::GetMaximumSize() const
    {
    // returning 0.0 signifies undisplayable tile...
    return IsDisplayable() ? s_tileScreenSize * GetZoomFactor() : 0.0;
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d TileTree::Tile::ComputeChildRange(Tile& child, bool is2d) const
    {
    // Each dimension of the relative ID is 0 or 1, indicating which bisection of the range to take
    TileId relativeId = child.GetRelativeTileId();
    BeAssert(2 > relativeId.m_i && 2 > relativeId.m_j && 2 > relativeId.m_k);

    // We should never subdivide along z for 2d tiles...Ideally we would use a quad-tree for those
    DRange3d range;
    if (is2d)
      {
      range = bisectRange2d(GetRange(), 0 == relativeId.m_i);
      range = bisectRange2d(range, 0 == relativeId.m_j);
      range = bisectRange2d(range, 0 == relativeId.m_k);
      }
    else
      {
      range = bisectRange(GetRange(), 0 == relativeId.m_i);
      range = bisectRange(range, 0 == relativeId.m_j);
      range = bisectRange(range, 0 == relativeId.m_k);
      }

    return range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d TileTree::Tile::GetDgnRange() const
    {
    DRange3d range;
    GetRoot().GetLocation().Multiply(range, GetTileRange());
    return range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TileTree::Tile::InitTolerance(double minToleranceRatio, bool isLeaf)
    {
    double diagDist = GetRoot().Is3d() ? m_range.DiagonalDistance() : m_range.DiagonalDistanceXY();
    m_tolerance = diagDist / (minToleranceRatio * GetZoomFactor());
    SetIsLeaf(isLeaf);
    BeAssert(0.0 != m_tolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileId TileId::GetRelativeId(TileId parentId) const
    {
    BeAssert(parentId.m_level+1 == m_level);
    return TileId(parentId.m_level, m_i % 2, m_j % 2, m_k % 2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileId TileTree::Tile::GetRelativeTileId() const
    {
    auto tileId = GetTileId();
    auto parent = GetParent();
    if (nullptr != parent)
        tileId = tileId.GetRelativeId(parent->GetTileId());

    return tileId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TilePtr TileTree::Tile::FindTile(TileId id, double zoomFactor)
    {
    if (id == GetTileId())
        return FindTile(zoomFactor);

    auto children = GetChildren(true);
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
TilePtr TileTree::Tile::FindTile(double zoomFactor)
    {
    if (zoomFactor == GetZoomFactor())
        return this;

    BeAssert(zoomFactor > 0.0 && HasZoomFactor());
    auto children = GetChildren(true);
    if (nullptr == children || 1 != children->size())
        {
        BeAssert(false);
        return nullptr;
        }

    return static_cast<TileR>(**children->begin()).FindTile(zoomFactor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TileTree::Tile::GetIdString() const
    {                                                                                                                                                                       
    // NB: Zoom factor is stored as a double for arithmetic purposes, but it's always an unsigned integral power of two.
    return Utf8PrintfString("%u/%u/%u/%u:%u", m_id.m_level, m_id.m_i, m_id.m_j, m_id.m_k, static_cast<uint32_t>(GetZoomFactor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileTree::Tile::ToJson(Json::Value& json) const
    {
    json["id"]["treeId"] = GetRoot().GetModelId().ToHexStr();
    json["id"]["tileId"] = GetIdString();
    json["maximumSize"] = IsDisplayable() ? s_tileScreenSize : 0.0;
    json["isLeaf"] = IsLeaf();

    JsonUtils::DRange3dToJson(json["range"], GetRange());
    if (HasContentRange())
        JsonUtils::DRange3dToJson(json["contentRange"], GetContentRange());

    if (HasZoomFactor())
        json["sizeMultiplier"] = GetZoomFactor();
    

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TilePtr TileTree::Tile::CreateChild(TileId childId) const
    {
    return TileTree::Tile::Create(const_cast<RootR>(GetRoot()), childId, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::WaitForAllLoadsFor(uint32_t milliseconds)
    {
    auto condition = [&](BeConditionVariable&) { return 0 == m_activeLoads.size(); };
    ConditionVariablePredicate<decltype(condition)> pred(condition);
    m_cv.WaitOnCondition(&pred, milliseconds);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::CancelAllTileLoads()
    {
    BeMutexHolder lock(m_cv.GetMutex());
    for (auto& load : m_activeLoads)
        load->SetCanceled();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::CancelTileLoad(TileCR tile)
    {
    // ###TODO_ELEMENT_TILE: Bentley containers don't support 'transparent' comparators, meaning we can't compare a TileLoadStateSPtr to a Tile even
    // though the comparator can. We should fix that - but for now, instead, we're using std::set.
    BeMutexHolder lock(m_cv.GetMutex());
    auto iter = m_activeLoads.find(&tile);
    if (iter != m_activeLoads.end())
        {
        (*iter)->SetCanceled();
        m_activeLoads.erase(iter);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
RootPtr Root::Create(GeometricModelR model, Render::SystemR system, bool asClassifier)
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

    RootPtr root = new Root(model, transform, system, asClassifier);
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
    m_rootTile = TileTree::Tile::CreateRoot(*this, range, populate);

    if (!populate)
        m_rootTile->SetIsReady();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TilePtr Root::FindTileById(Utf8CP strId)
    {
    if (GetRootTile().IsNull())
        return nullptr;
    
    // NB: Zoom factor is stored as a double for arithmetic purposes, but it's always an unsigned integral power of two.
    uint32_t zoomFactor;
    TileId id;
    if (nullptr == strId || 5 != BE_STRING_UTILITIES_UTF8_SSCANF(strId, "%" SCNu8 "/%u/%u/%u:%u", &id.m_level, &id.m_i, &id.m_j, &id.m_k, &zoomFactor))
        {
        BeAssert(false && "Invalid tile id string");
        return nullptr;
        }

    return GetRootTile()->FindTile(id, static_cast<double>(zoomFactor));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Render::SystemR TileTree::Tile::GetRenderSystem() const { return m_root.GetRenderSystem(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value TileMetadata::ToJson() const
    {
    Json::Value json;
    json["zoomFactor"] = m_zoomFactor;
    json["flags"] = static_cast<uint32_t>(m_flags);
    m_contentRange.ToJson(json["contentRange"]);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
void TileMetadata::FromJson(Json::Value const& json)
    {
    m_zoomFactor = json["zoomFactor"].asDouble();
    m_flags = static_cast<TileFlags>(json["flags"].asUInt());
    m_contentRange.FromJson(json["contentRange"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Loader::_GetFromSource()
    {
    TileR   tile = *m_tile;
    RootR   root = tile.GetRoot();

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
        bool isLeaf = tile.IsLeaf();
        double zoomFactor = tile.GetZoomFactor();
        if (SUCCESS != Dgn::Tile::IO::WriteDgnTile (m_tileBytes, tile.GetContentRange(), geometry, *root.GetModel(), isLeaf, tile.HasZoomFactor() ? &zoomFactor : nullptr))
            return ERROR;

        m_tileMetadata.SetContentRange(tile.GetContentRange());
        m_tileMetadata.SetIsDisplayable(true);
        m_tileMetadata.SetIsLeaf(isLeaf);
        m_tileMetadata.SetContainsCurves(geometry.ContainsCurves());
        m_tileMetadata.SetIsIncomplete(!geometry.IsComplete());
        if (tile.HasZoomFactor())
            m_tileMetadata.SetZoomFactor(tile.GetZoomFactor());
        }
    
    m_saveToCache = true;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Loader::_LoadTile() 
    { 
    TileR                                   tile = *m_tile;
    RootR                                   root = tile.GetRoot();
    ElementAlignedBox3d                     contentRange;
    StopWatch                               stopWatch(true);

    bool isLeafInCache = false;
    auto& geometry = m_geometry;
    // NB: If we loaded this tile from the cache, m_saveToCache will be false. Read it from the cache data.
    // Otherwise, we've already populated m_geometry from model and written it to m_tileBytes. Do not deserialize it again.
    if (!m_tileBytes.empty() && !m_saveToCache)
        {
        BeAssert(geometry.IsEmpty());
        if (Dgn::Tile::IO::ReadStatus::Success != Dgn::Tile::IO::ReadDgnTile (contentRange, geometry, m_tileBytes, *root.GetModel(), GetRenderSystem(), isLeafInCache, tile.GetRange()))
            {
            BeAssert(false);
            return ERROR;
            }
        }

    tile.SetContentRange(contentRange);

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

    bool haveGeometry = !geometry.Meshes().empty();
    return tile.GetRoot().UnderMutex([&]()
        {
        // Possible that one or more parent tiles will contain solely elements too small to produce geometry, in which case
        // we must create their children in order to get graphics...mark undisplayable.
        tile.SetDisplayable(haveGeometry);
        return SUCCESS;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Loader::LoadGeometryFromModel()
    {
#if defined (BENTLEYCONFIG_PARASOLID)    
    PSolidThreadUtil::WorkerThreadOuterMark outerMark;
#endif

    auto& tile = *m_tile;

    LoadContext loadContext(this);
    m_geometry = generateTileGeometry(tile, loadContext);

    return loadContext.WasAborted() ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileLoaderPtr TileTree::Tile::CreateTileLoader(TileLoadStateSPtr loads)
    {
    return Loader::Create(*this, loads);
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
    GeomPartPtr tileGeomPart = generateGeomPart(partId, geomParams, *this);
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
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
MeshGenerator::MeshGenerator(TileCR tile, GeometryOptionsCR options, LoadContextCR loadContext)
  : m_tile(tile), m_options(options), m_tolerance(tile.GetTolerance()), m_loadContext(loadContext),
    m_featureTable(tile.GetRoot().GetModelId(), loadContext.GetRenderSystem()._GetMaxFeaturesPerBatch()),
    m_builderMap(m_tolerance, &m_featureTable, tile.GetTileRange(), m_tile.GetRoot().Is2d())
    {
    SetDgnDb(m_tile.GetRoot().GetDgnDb());
    m_is3dView = m_tile.GetRoot().Is3d();
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
    double rangePixels = (m_tile.GetRoot().Is3d() ? geomRange.DiagonalDistance() : geomRange.DiagonalDistanceXY()) / m_tolerance;
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
    double rangePixels = (m_tile.GetRoot().Is3d() ? geomRange.DiagonalDistance() : geomRange.DiagonalDistanceXY()) / m_tolerance;
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
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddPolyface(Polyface& tilePolyface, GeometryR geom, double rangePixels, bool isContained)
    {
    // TFS#817210
    PolyfaceHeaderPtr polyface = tilePolyface.m_polyface.get();
    if (polyface.IsNull() || 0 == polyface->GetPointIndexCount())
        return;


    if (nullptr != m_tile.GetRoot().GetPreprocessor())
        m_tile.GetRoot().GetPreprocessor()->Preprocess(polyface);

    bool doDecimate = !m_tile.IsLeaf() && !m_tile.HasZoomFactor() && geom.DoDecimate() && polyface->GetPointCount() > GetDecimatePolyfacePointCount() && 0 == polyface->GetFaceCount();

    if (doDecimate)
        {
        BeAssert(0 == polyface->GetEdgeChainCount());       // The decimation does not handle edge chains - but this only occurs for polyfaces which should never have them.
        PolyfaceHeaderPtr   decimated;
        if (doDecimate && (decimated = polyface->ClusteredVertexDecimate(m_tolerance, .25 /* No decimation unless point count reduced by at least 25% */)).IsValid())
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
    DgnDbR              db = m_tile.GetRoot().GetDgnDb();
    MeshAuxData         auxData;
    uint64_t            keyElementId = (nullptr != m_tile.GetRoot().GetPreprocessor() && m_tile.GetRoot().GetPreprocessor()->SeparatePrimitivesById()) ? elemId.GetValue() : 0;    // Create seperate primitives per element if Classifying only.
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeshGenerator::ReadFrom(StreamBufferR stream, bool& containsCurves, bool& isIncomplete, DgnElementIdSet const& omitElems)
    {
    auto model = m_tile.GetRoot().GetModel();
    BeAssert(model.IsValid());
    BeAssert(nullptr != _GetRenderSystem());

    Dgn::Tile::IO::DgnTile::Flags tileFlags;

    auto status = Dgn::Tile::IO::ReadDgnTile(m_builderMap, stream, *model, *_GetRenderSystem(), tileFlags, omitElems);
    if (Dgn::Tile::IO::ReadStatus::Success != status)
        {
        BeAssert(false);
        return false;
        }

    containsCurves = Dgn::Tile::IO::DgnTile::Flags::None != (tileFlags & Dgn::Tile::IO::DgnTile::Flags::ContainsCurves);
    isIncomplete = Dgn::Tile::IO::DgnTile::Flags::None != (tileFlags & Dgn::Tile::IO::DgnTile::Flags::Incomplete);

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
        if (aborted)
            break;
        }

    if (m_meshGenerator.DidDecimation())
        m_geometries.MarkIncomplete();

    if (loadContext.WasAborted())
        {
        m_geometries.clear();
        return Completion::Aborted;
        }

    // Determine whether or not to subdivide this tile
    bool canSkipSubdivision = tile.GetTolerance() <= s_maxLeafTolerance;
    if (canSkipSubdivision && !loadContext.WasAborted() && !tile.IsLeaf() && !tile.HasZoomFactor())
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


    // Facet all geometry thus far collected to produce meshes.
    Render::Primitives::GeometryCollection collection;
    collection.Meshes() = m_meshGenerator.GetMeshes();
    tile.SetContentRange(ElementAlignedBox3d(m_meshGenerator.GetContentRange()));
    if (!m_geometries.IsComplete())
        collection.MarkIncomplete();

    if (m_geometries.ContainsCurves())
        collection.MarkCurved();

    output = std::move(collection);

    m_geometries.clear(); // NB: Retains curved/complete flags...

    return Completion::Full;
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
        m_curElemId = elemId;
        m_curRangeDiagonalSquared = rangeDiagonalSquared;
        VisitElement(elemId, false);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationPreprocessor::ClassificationPreprocessor(RootCR root)
    {
    Transform transformFromDgn;
    transformFromDgn.InverseOf(root.GetLocation());

    transformFromDgn.Multiply(m_range, root.GetDgnDb().GeoLocation().GetProjectExtents());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassificationPreprocessor::Preprocess(CurveVectorPtr& curveVector)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassificationPreprocessor::Preprocess(PolyfaceHeaderPtr& polyface)
    {
    DPlane3d        plane;

    if (polyface->IsClosedPlanarRegion(plane))
        {
        DRay3d              ray = DRay3d::FromOriginAndVector(plane.origin, plane.normal);
        DRange1d            classifiedProjection = m_range.GetCornerRange(ray);
        PolyfaceHeaderPtr   extrudedPolyface = polyface->ComputeOffset(PolyfaceHeader::OffsetOptions(), classifiedProjection.high, classifiedProjection.low);

        if (extrudedPolyface.IsValid())
            {
            extrudedPolyface->Triangulate();
            extrudedPolyface->BuildPerFaceNormals();
            polyface = extrudedPolyface;
            }
        }
    }






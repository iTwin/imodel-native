/*--------------------------------------------------------------------------------------+                                                                                           
|
|     $Source: DgnCore/ElementTileTree.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/ElementTileTree.h>
#include <DgnPlatform/TileIO.h>
#include <folly/BeFolly.h>
#include <DgnPlatform/RangeIndex.h>
#if defined (BENTLEYCONFIG_PARASOLID) 
#include <DgnPlatform/DgnBRep/PSolidUtil.h>
#endif

// Define this if you want to generate a root tile containing geometry.
// By default the root tile is empty.
// #define POPULATE_ROOT_TILE

USING_NAMESPACE_ELEMENT_TILETREE
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES

using FBox3d = RangeIndex::FBox;

BEGIN_UNNAMED_NAMESPACE

struct TileContext;

#if defined (BENTLEYCONFIG_PARASOLID) 

// The ThreadLocalParasolidHandlerStorageMark sets up the local storage that will be used 
// by all threads.

typedef RefCountedPtr <struct ThreadedParasolidErrorHandlerInnerMark>     ThreadedParasolidErrorHandlerInnerMarkPtr;


class   ParasolidException {};

//#define REALITY_CACHE_SUPPORT



/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley      10/2015
*  Called from the main thread to register Thread Local Storage used by
*  all threads for Parasolid error handling.
+===============+===============+===============+===============+===============+======*/
struct  ThreadedLocalParasolidHandlerStorageMark
{
    BeThreadLocalStorage*       m_previousLocalStorage;

    ThreadedLocalParasolidHandlerStorageMark ();
    ~ThreadedLocalParasolidHandlerStorageMark ();
};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley      10/2015
*  Inner mark.   Included around code sections that should be rolled back in case
*                Of serious error.
+===============+===============+===============+===============+===============+======*/
struct  ThreadedParasolidErrorHandlerInnerMark : RefCountedBase
{
    static ThreadedParasolidErrorHandlerInnerMarkPtr Create () { return new ThreadedParasolidErrorHandlerInnerMark(); }                                                 

protected:

    ThreadedParasolidErrorHandlerInnerMark();
    ~ThreadedParasolidErrorHandlerInnerMark();
};
      
typedef RefCountedPtr <struct ThreadedParasolidErrorHandlerOuterMark>     ThreadedParasolidErrorHandlerOuterMarkPtr;

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2015
*  Outer mark.   Included once to set up Parasolid error handling for a single thread.
+===============+===============+===============+===============+===============+======*/
struct  ThreadedParasolidErrorHandlerOuterMark  : RefCountedBase 
{
    PK_ERROR_frustrum_t     m_previousErrorFrustum;

    static ThreadedParasolidErrorHandlerOuterMarkPtr Create () { return new ThreadedParasolidErrorHandlerOuterMark(); }

protected:

    ThreadedParasolidErrorHandlerOuterMark();
    ~ThreadedParasolidErrorHandlerOuterMark();
};
    

static      BeThreadLocalStorage*       s_threadLocalParasolidHandlerStorage;    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      10/2015                                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ThreadedLocalParasolidHandlerStorageMark::ThreadedLocalParasolidHandlerStorageMark ()
    {
    if (nullptr == (m_previousLocalStorage = s_threadLocalParasolidHandlerStorage))
        s_threadLocalParasolidHandlerStorage = new BeThreadLocalStorage;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ThreadedLocalParasolidHandlerStorageMark::~ThreadedLocalParasolidHandlerStorageMark () 
    { 
    if (nullptr == m_previousLocalStorage) 
        DELETE_AND_CLEAR (s_threadLocalParasolidHandlerStorage);

    }

typedef bvector<PK_MARK_t>  T_RollbackMarks;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static T_RollbackMarks*  getRollbackMarks () 
    { 
    static T_RollbackMarks      s_unthreadedMarks;

    return nullptr == s_threadLocalParasolidHandlerStorage ? &s_unthreadedMarks : reinterpret_cast <T_RollbackMarks*> (s_threadLocalParasolidHandlerStorage->GetValueAsPointer());   
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void clearRollbackMarks () 
    {    
    T_RollbackMarks*    rollbackMarks;
             
    if (nullptr != s_threadLocalParasolidHandlerStorage &&
        nullptr != (rollbackMarks = reinterpret_cast <T_RollbackMarks*> (s_threadLocalParasolidHandlerStorage->GetValueAsPointer())))
        {
        delete rollbackMarks;
        s_threadLocalParasolidHandlerStorage->SetValueAsPointer(nullptr);
        } 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void clearExclusions()
    {
    PK_THREAD_exclusion_t       clearedExclusion;
    PK_LOGICAL_t                clearedThisThread;

    PK_THREAD_clear_exclusion (PK_THREAD_exclusion_serious_c, &clearedExclusion, &clearedThisThread);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static PK_ERROR_code_t threadedParasolidErrorHandler (PK_ERROR_sf_t* errorSf)
    {
    if (errorSf->severity > PK_ERROR_mild)
        {
        switch (errorSf->code)
            {
            case 942:         // Edge crossing (constructing face from curve vector to perform intersections)
            case 547:         // Nonmanifold  (constructing face from curve vector to perform intersections)
            case 1083:        // Degenerate trim loop.
                break;

            default:
                printf ("Error %d caught in parasolid error handler\n", errorSf->code);
                BeAssert (false && "Severe error during threaded processing");
                break;
            }
        PK_MARK_goto (getRollbackMarks()->back());
        clearExclusions ();
        
        PK_THREAD_tidy();

        throw ParasolidException();
        }
    
    return 0; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ThreadedParasolidErrorHandlerOuterMark::ThreadedParasolidErrorHandlerOuterMark ()
    {
    BeAssert (nullptr == getRollbackMarks());      // The outer mark is not nestable.

    PK_THREAD_ask_error_cbs (&m_previousErrorFrustum);

    PK_ERROR_frustrum_t     errorFrustum;

    errorFrustum.handler_fn = threadedParasolidErrorHandler;
    PK_THREAD_register_error_cbs (errorFrustum);

    s_threadLocalParasolidHandlerStorage->SetValueAsPointer (new T_RollbackMarks());
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ThreadedParasolidErrorHandlerOuterMark::~ThreadedParasolidErrorHandlerOuterMark ()
    {
    PK_THREAD_register_error_cbs (m_previousErrorFrustum);
    clearRollbackMarks(); 
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ThreadedParasolidErrorHandlerInnerMark::ThreadedParasolidErrorHandlerInnerMark ()
    {
    PK_MARK_t       mark;

    PK_MARK_create (&mark);
    getRollbackMarks()->push_back (mark);
    }
      
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ThreadedParasolidErrorHandlerInnerMark::~ThreadedParasolidErrorHandlerInnerMark ()
    {
    PK_MARK_delete (getRollbackMarks()->back());
    getRollbackMarks()->pop_back();
    }

#endif

constexpr double s_minRangeBoxSize    = 5.0;     // Threshold below which we consider geometry/element too small to contribute to tile mesh
constexpr double s_tileScreenSize = 512.0;
constexpr double s_minToleranceRatio = s_tileScreenSize;
constexpr uint32_t s_minElementsPerTile = 100;
constexpr double s_solidPrimitivePartCompareTolerance = 1.0E-5;
constexpr double s_spatialRangeMultiplier = 1.0;
constexpr uint32_t s_hardMaxFeaturesPerTile = 2048*1024;

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
    TileContext&        m_context;
    double              m_rangeDiagonalSquared;

    void _AddPolyface(PolyfaceQueryCR, bool) override;
    void _AddPolyfaceR(PolyfaceHeaderR, bool) override;
    void _AddTile(TextureCR tx, TileCorners const& corners) override;
    void _AddSubGraphic(GraphicR, TransformCR, GraphicParamsCR, ClipVectorCP) override;
    bool _WantStrokeLineStyle(LineStyleSymbCR, IFacetOptionsPtr&) override;
    GraphicBuilderPtr _CreateSubGraphic(TransformCR, ClipVectorCP) const override;
    GraphicPtr _FinishGraphic(GeometryAccumulatorR) override;

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

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct TileContext : NullContext
{
    enum class Result { Success, NoGeometry, Aborted };
private:
    IFacetOptionsR                  m_facetOptions;
    mutable IFacetOptionsPtr        m_lsStrokerOptions;
    RootR                           m_root;
    GeometryList&                   m_geometries;
    DRange3d                        m_range;
    DRange3d                        m_tileRange;
    BeSQLite::CachedStatementPtr    m_statement;
    LoadContextCR                   m_loadContext;
    Transform                       m_transformFromDgn;
    double                          m_minRangeDiagonalSquared;
    double                          m_minTextBoxSize;
    double                          m_tolerance;
    GraphicPtr                      m_finishedGraphic;
    TileBuilderPtr                  m_tileBuilder;
    TileSubGraphicPtr               m_subGraphic;
    DgnElementId                    m_curElemId;
    double                          m_curRangeDiagonalSquared;
    bool                            m_wantCacheSolidPrimitives = false;
protected:
    void PushGeometry(GeometryR geom);

    TileContext(GeometryList& geoms, RootR root, DRange3dCR range, IFacetOptionsR facetOptions, TransformCR transformFromDgn, double tileTolerance, double rangeTolerance, LoadContextCR loadContext);

    Render::GraphicBuilderPtr _CreateGraphic(Render::GraphicBuilder::CreateParams const& params) override
        {
        Render::GraphicBuilderPtr builder(m_tileBuilder);
        if (builder->GetRefCount() > 2)
            builder = new TileBuilder(*this, m_curElemId, m_curRangeDiagonalSquared, params);
        else
            m_tileBuilder->ReInitialize(m_curElemId, m_curRangeDiagonalSquared, params.m_placement);

        BeAssert(builder->GetRefCount() <= 2);
        return builder;
        }

    bool IsValueNull(int index) { return m_statement->IsColumnNull(index); }
    StatusInt _VisitElement(DgnElementId elementId, bool allowLoad) override;
    Render::GraphicPtr _StrokeGeometry(GeometrySourceCR, double) override;
    void _AddSubGraphic(Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, GeometryParamsR geomParams) override;
    bool _CheckStop() override { return WasAborted() || AddAbortTest(m_loadContext.WasAborted()); }

    Render::MaterialPtr _GetMaterial(DgnMaterialId id) const override
        {
        Render::SystemP system = m_loadContext.GetRenderSystem();
        return nullptr != system ? system->_GetMaterial(id, GetDgnDb()) : nullptr;
        }

    static Render::ViewFlags GetDefaultViewFlags();
public:
    TileContext(GeometryList& geometries, RootR root, DRange3dCR range, IFacetOptionsR facetOptions, TransformCR transformFromDgn, double tolerance, LoadContextCR loadContext)
        : TileContext(geometries, root, range, facetOptions, transformFromDgn, tolerance, tolerance, loadContext) { }

    void ProcessElement(DgnElementId elementId, double diagonalRangeSquared);
    void AddGeomPart (Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, GeometryParamsR geomParams, GraphicParamsR graphicParams);
    GeomPartPtr GenerateGeomPart(DgnGeometryPartCR, GeometryParamsR);

    RootR GetRoot() const { return m_root; }
    System& GetRenderSystem() const { BeAssert(nullptr != m_loadContext.GetRenderSystem()); return *m_loadContext.GetRenderSystem(); }

    double GetMinRangeDiagonalSquared() const { return m_minRangeDiagonalSquared; }
    bool BelowMinRange(DRange3dCR range) const
        {
        // Avoid processing any elements with range smaller than roughly half a pixel...
        auto diag = range.low.DistanceSquared(range.high);
        return diag < m_minRangeDiagonalSquared && 0.0 < diag; // ###TODO_ELEMENT_TILE: Dumb single-point primitives...
        }

    size_t GetGeometryCount() const { return m_geometries.size(); }
    void TruncateGeometryList(size_t maxSize) { m_geometries.resize(maxSize); m_geometries.MarkIncomplete(); }

    IFacetOptionsPtr GetLineStyleStrokerOptions(LineStyleSymbCR lsSymb) const
        {
        if (!lsSymb.GetUseStroker())
            return nullptr;

        // Only stroke if line width at least 5 pixels...
        double pixelSize = m_tolerance;
        double maxWidth = lsSymb.GetStyleWidth();
        constexpr double pixelThreshold = 5.0;

        if (0.0 != pixelSize && maxWidth / pixelSize < pixelThreshold)
            return nullptr;

        if (m_lsStrokerOptions.IsNull())
            {
            // NB: During geometry collection, tolerance is generally set for leaf node
            // We don't apply facet options tolerances until we convert the geometry to meshes/strokes
            m_lsStrokerOptions = IFacetOptions::CreateForCurves();
            m_lsStrokerOptions->SetAngleTolerance(Angle::FromDegrees(5.0).Radians());
            }

        return m_lsStrokerOptions;
        }

    DgnElementId GetCurrentElementId() const { return m_curElemId; }
    TransformCR GetTransformFromDgn() const { return m_transformFromDgn; }
    IFacetOptionsR GetFacetOptions() { return m_facetOptions; }

    GraphicPtr FinishGraphic(GeometryAccumulatorR accum, TileBuilder& builder);
    GraphicPtr FinishSubGraphic(GeometryAccumulatorR accum, TileSubGraphic& subGf);

    void MarkIncomplete() { m_geometries.MarkIncomplete(); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileBuilder::TileBuilder(TileContext& context, DgnElementId elemId, double rangeDiagonalSquared, CreateParams const& params)
    : GeometryListBuilder(context.GetRenderSystem(), params, elemId, context.GetTransformFromDgn()), m_context(context), m_rangeDiagonalSquared(rangeDiagonalSquared)
    {
    SetCheckGlyphBoxes(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileBuilder::TileBuilder(TileContext& context, DRange3dCR range)
    : GeometryListBuilder(context.GetRenderSystem(), CreateParams(context.GetDgnDb())), m_context(context), m_rangeDiagonalSquared(range.low.DistanceSquared(range.high))
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
    m_rangeDiagonalSquared = range.low.DistanceSquared(range.high);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TileBuilder::_AddPolyface(PolyfaceQueryCR geom, bool filled)
    {
    size_t maxPerFace;
    PolyfaceHeaderPtr polyface;
    auto& facetOptions = m_context.GetFacetOptions();
    if ((facetOptions.GetNormalsRequired() && 0 == geom.GetNormalCount()) ||
        (facetOptions.GetParamsRequired() && (0 == geom.GetParamCount() || 0 == geom.GetFaceCount())) ||
        (geom.GetNumFacet(maxPerFace) > 0 && (int) maxPerFace > facetOptions.GetMaxPerFace()))
        {
        IPolyfaceConstructionPtr builder = PolyfaceConstruction::New(facetOptions);
        builder->AddPolyface(geom);
        polyface = &builder->GetClientMeshR();
        }
    else
        {
        polyface = geom.Clone();
        }

    Add(*polyface, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TileBuilder::_AddPolyfaceR(PolyfaceHeaderR geom, bool filled)
    {
    size_t maxPerFace;
    auto& facetOptions = m_context.GetFacetOptions();
    if ((facetOptions.GetNormalsRequired() && 0 == geom.GetNormalCount()) ||
        (facetOptions.GetParamsRequired() && (0 == geom.GetParamCount() || 0 == geom.GetFaceCount())) ||
        (geom.GetNumFacet(maxPerFace) > 0 && (int) maxPerFace > facetOptions.GetMaxPerFace()))
        {
        IPolyfaceConstructionPtr builder = PolyfaceConstruction::New(facetOptions);
        builder->AddPolyface(geom);
        Add(builder->GetClientMeshR(), filled);
        }
    else
        {
        Add(geom, filled);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TileBuilder::_AddTile(TextureCR tx, TileCorners const& corners)
    {
    // ###TODO_ELEMENT_TILE: tri mesh w/ texture...
    DPoint3d    shapePoints[5];

    shapePoints[0] = shapePoints[4] = corners.m_pts[0];
    shapePoints[1] = corners.m_pts[1];
    shapePoints[2] = corners.m_pts[2];
    shapePoints[3] = corners.m_pts[3];

    _AddShape(5, shapePoints, true);
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
    CreateParams params(GetDgnDb(), Transform::FromProduct(GetLocalToWorldTransform(), tf));
    TileBuilderPtr subGf = new TileBuilder(m_context, GetElementId(), m_rangeDiagonalSquared, params);
    subGf->ActivateGraphicParams(GetGraphicParams(), GetGeometryParams());
    return subGf.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicPtr TileBuilder::_FinishGraphic(GeometryAccumulatorR accum)
    {
    return m_context.FinishGraphic(accum, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileBuilder::_WantStrokeLineStyle(LineStyleSymbCR symb, IFacetOptionsPtr& options)
    {
    options = m_context.GetLineStyleStrokerOptions(symb);
    return options.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileSubGraphic::TileSubGraphic(TileContext& context, DgnGeometryPartCP part)
    : TileBuilder(context, nullptr != part ? part->GetBoundingBox() : DRange3d::NullRange()), m_input(part)
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
    m_tileBuilder = new TileBuilder(*this, DgnElementId(), 0.0, GraphicBuilder::CreateParams(root.GetDgnDb()));
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
    bool            m_aborted = false;
    bool            m_anySkipped = false;

    bool CheckStop() { return (m_aborted = m_aborted && m_loadContext.WasAborted()); }
    void Insert(double diagonalSq, DgnElementId elemId)
        {
        m_entries.Insert(diagonalSq, elemId);
        if (m_entries.size() > m_maxElements)
            {
            m_entries.erase(--m_entries.end()); // remove the smallest element.
            m_anySkipped = true;
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

        double sizeSq = entry.m_range.m_low.DistanceSquared(entry.m_range.m_high);
        if (sizeSq >= m_minRangeDiagonalSquared)
            Insert(sizeSq, entry.m_id);
        else
            m_anySkipped = true;
        
        return Stop::No;
        }
public:
    ElementCollector(DRange3dCR range, RangeIndex::Tree& rangeIndex, double minRangeDiagonalSquared, LoadContextCR loadContext, uint32_t maxElements)
        : m_range(range), m_minRangeDiagonalSquared(minRangeDiagonalSquared), m_maxElements(maxElements), m_loadContext(loadContext)
        {
        rangeIndex.Traverse(*this);
        }

    bool AnySkipped() const { return m_anySkipped; }
    Entries const& GetEntries() const { return m_entries; }
    double ComputeLeafTolerance() const;
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

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Loader::Loader(TileR tile, TileTree::TileLoadStatePtr loads, Dgn::Render::SystemP renderSys)
    : T_Super("", tile, loads, "", renderSys)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> Loader::_GetFromSource()
    {
    LoaderPtr me(this);
    return folly::via(&BeFolly::ThreadPool::GetCpuPool(), [me]() { return me->DoGetFromSource(); });
    }



#ifdef REALITY_CACHE_SUPPORT

#define POPULATE_ROOT_TILE      // Fow now - easier to debug..
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Loader::_LoadTile() 
    { 
    TileR   tile = static_cast<TileR> (*m_tile);
    RootR   root = tile.GetElementRoot();

    Render::Primitives::GeometryCollection geometry;

    if (SUCCESS != TileTree::TileIO::ReadTile (geometry, m_tileBytes, *root.GetModel()))
        return ERROR;

    // No point subdividing empty nodes - improves performance if we don't
    // Also not much point subdividing nodes containing no curved geometry
    // NB: We cannot detect either of the above if any elements or geometry were skipped during tile generation.
    if (geometry.IsComplete())
        {
        if (geometry.IsEmpty() || !geometry.ContainsCurves())
            tile.SetIsLeaf();
        }

    auto  system = GetRenderSystem();
    if (nullptr == system)
        {
        // This is checked in _CreateTileTree()...
        BeAssert(false && "ElementTileTree requires a Render::System");
        return ERROR;
        }

    GetMeshGraphicsArgs             args;
    bvector<Render::GraphicPtr>     graphics;

    for (auto const& mesh : geometry.Meshes())
        mesh->GetGraphics (graphics, *system, args, root.GetDgnDb());

    tile.SetIsReady();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Loader::DoGetFromSource()
    {
#if defined (BENTLEYCONFIG_PARASOLID) 
    ThreadedLocalParasolidHandlerStorageMark  parasolidParasolidHandlerStorageMark;
    PSolidKernelManager::StartSession();
    ThreadedParasolidErrorHandlerOuterMarkPtr  outerMark = ThreadedParasolidErrorHandlerOuterMark::Create();
    ThreadedParasolidErrorHandlerInnerMarkPtr  innerMark = ThreadedParasolidErrorHandlerInnerMark::Create(); 
#endif

    auto& tile = static_cast<TileR>(*m_tile);
    RootR root = tile.GetElementRoot();

    auto  system = GetRenderSystem();
    if (nullptr == system)
        {
        // This is checked in _CreateTileTree()...
        BeAssert(false && "ElementTileTree requires a Render::System");
        return ERROR;
        }

    LoadContext loadContext(this);
    auto geometry = tile.GenerateGeometry(loadContext);

    if (loadContext.WasAborted())
        return ERROR;
        
    return TileTree::TileIO::WriteTile (m_tileBytes, geometry, *root.GetModel(), tile.GetCenter());     // TBD -- Avoid round trip through m_tileBytes when loading from elements.
    }


#else
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Loader::DoGetFromSource()
    {
    return IsCanceledOrAbandoned() ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Loader::_LoadTile()
    {
#if defined (BENTLEYCONFIG_PARASOLID) 
    ThreadedLocalParasolidHandlerStorageMark  parasolidParasolidHandlerStorageMark;
    PSolidKernelManager::StartSession();
    ThreadedParasolidErrorHandlerOuterMarkPtr  outerMark = ThreadedParasolidErrorHandlerOuterMark::Create();
    ThreadedParasolidErrorHandlerInnerMarkPtr  innerMark = ThreadedParasolidErrorHandlerInnerMark::Create(); 
#endif

    auto& tile = static_cast<TileR>(*m_tile);
    RootR root = tile.GetElementRoot();

    auto  system = GetRenderSystem();
    if (nullptr == system)
        {
        // This is checked in _CreateTileTree()...
        BeAssert(false && "ElementTileTree requires a Render::System");
        return ERROR;
        }

    LoadContext loadContext(this);
    auto geometry = tile.GenerateGeometry(loadContext);

    if (loadContext.WasAborted())
        return ERROR;

    // No point subdividing empty nodes - improves performance if we don't
    // Also not much point subdividing nodes containing no curved geometry
    // NB: We cannot detect either of the above if any elements or geometry were skipped during tile generation.
    if (geometry.IsComplete())
        {
        if (geometry.IsEmpty() || !geometry.ContainsCurves())
            tile.SetIsLeaf();
        }

    GetMeshGraphicsArgs             args;
    bvector<Render::GraphicPtr>     graphics;

    for (auto const& mesh : geometry.Meshes())
        mesh->GetGraphics (graphics, *system, args, root.GetDgnDb());

    if (!graphics.empty())
        {
        GraphicPtr graphic;
        switch (graphics.size())
            {
            case 0:
                break;
            case 1:
                graphic = *graphics.begin();
                break;
            default:
                graphic = system->_CreateGraphicList(std::move(graphics), root.GetDgnDb());
                break;
            }

        if (graphic.IsValid())
            tile.SetGraphic(*system->_CreateBatch(*graphic, std::move(geometry.Meshes().m_features)));
        }

    tile.SetIsReady();
    return SUCCESS;
    }
#endif


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Root::Root(GeometricModelR model, TransformCR transform, Render::SystemR system) : T_Super(model.GetDgnDb(), transform, "", &system),
    m_modelId(model.GetModelId()), m_name(model.GetName()), m_is3d(model.Is3dModel()), m_cacheGeometry(m_is3d)
    {
    // ###TODO: Play with this? Default of 20 seconds is ok for reality tiles which are cached...pretty short for element tiles.
    SetExpirationTime(BeDuration::Seconds(90));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
RootPtr Root::Create(GeometricModelR model, Render::SystemR system)
    {
    DgnDb::VerifyClientThread();

    if (DgnDbStatus::Success != model.FillRangeIndex())
        return nullptr;

    DRange3d range;
    bool populateRootTile;
    if (model.Is3dModel())
        {
        range = model.GetDgnDb().GeoLocation().GetProjectExtents();

        // This drastically reduces the time required to generate the root tile, so that the user sees *something* on the screen much sooner.
        range.ScaleAboutCenter(range, s_spatialRangeMultiplier);
        populateRootTile = isElementCountLessThan(s_minElementsPerTile, *model.GetRangeIndex());
        }
    else
        {
        // ###TODO_ELEMENT_TILE: What happens if user later adds geometry outside initial range?
        RangeAccumulator accum(range, model.Is2dModel());
        if (!accum.Accumulate(*model.GetRangeIndex()))
            {
            // return nullptr; ###TODO_ELEMENT_TILE: Empty models exist...
            range = DRange3d::From(DPoint3d::FromZero());
            }

        populateRootTile = accum.GetElementCount() < s_minElementsPerTile;
        }

#if defined(POPULATE_ROOT_TILE)
    // For debugging...
    populateRootTile = true;
#endif

    // Translate world coordinates to center of range in order to reduce precision errors
    DPoint3d centroid = DPoint3d::FromInterpolate(range.low, 0.5, range.high);
    Transform transform = Transform::From(centroid);

    // ###TODO parasolid...
#if defined (BENTLEYCONFIG_PARASOLID) 
    ThreadedLocalParasolidHandlerStorageMark  parasolidParasolidHandlerStorageMark;
    PSolidKernelManager::StartSession();
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
    part->SetInCache(true);

    m_mutex.lock(); // << LOCK

    BeAssert(m_parts.end() == m_parts.find(partId));
    m_parts.Insert(partId, part);

    foundBuilder = m_builders.find(partId);
    BeAssert(m_builders.end() != foundBuilder);
    m_builders.erase(foundBuilder);

    m_mutex.unlock(); // >> UNLOCK
    builder->GetMutex().unlock();
    builder->NotifyAll();

    return part;
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
    geomList.push_back(*Geometry::Create(*geom, Transform::FromIdentity(), range, elemId, displayParams, prim.HasCurvedFaceOrEdge(), db));
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
    double diag = range.low.DistanceSquared(range.high); // ###TODO: Cache this.
    if (0.0 == diag)
        return false;

    BeAssert(m_is3d); // we only bother caching for 3d...want rangeRatio relative to actual range, not expanded range
    diag /= s_spatialRangeMultiplier;
    return rangeDiagSq / diag >= rangeRatio;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::AddCachedGeometry(GeometryList&& geometry, DgnElementId elementId, double rangeDiagSq) const
    {
    if (!WantCacheGeometry(rangeDiagSq))
        return;

    BeMutexHolder lock(m_mutex);
    auto pair = m_geomLists.Insert(elementId, std::move(geometry));
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
    else if (m_debugGraphics.m_options == options)
        return m_debugGraphics.m_graphic;

    m_debugGraphics.m_options = options;

    bool wantRange = Root::DebugOptions::None != (Root::DebugOptions::ShowBoundingVolume & options);
    bool wantContentRange = Root::DebugOptions::None != (Root::DebugOptions::ShowContentVolume & options);
    if (!wantRange && !wantContentRange)
        return (m_debugGraphics.m_graphic = nullptr);

    GraphicBuilderPtr gf = GetElementRoot().GetRenderSystemP()->_CreateGraphic(GraphicBuilder::CreateParams(GetElementRoot().GetDgnDb()));
    GraphicParams params;
    params.SetWidth(0);
    if (wantRange)
        {
        bool isLeaf = IsLeaf() || HasZoomFactor();
        ColorDef color = isLeaf ? ColorDef::DarkBlue() : ColorDef::DarkOrange();
        params.SetLineColor(color);
        params.SetFillColor(color);
        params.SetLinePixels(isLeaf ? LinePixels::Code5 : LinePixels::Code4);
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
    return Utf8PrintfString("%d/%d/%d/%d:%f", m_id.m_level, m_id.m_i, m_id.m_j, m_id.m_k, m_zoomFactor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Tile::_DrawGraphics(TileTree::DrawArgsR args) const
    {
    T_Super::_DrawGraphics(args);
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
        m_range = ElementAlignedBox3d(parent->ComputeChildRange(*this));
    else
        m_range.Extend(*range);

    InitTolerance();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::Tile(Tile const& parent) : T_Super(const_cast<Root&>(parent.GetElementRoot()), parent.GetTileId(), &parent, false)
    {
    m_range.Extend(parent.GetRange());

    BeAssert(parent.HasZoomFactor());
    SetZoomFactor(parent.GetZoomFactor() * 2.0);

    InitTolerance();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Tile::InitTolerance()
    {
    m_tolerance = m_range.DiagonalDistance() / (s_minToleranceRatio * m_zoomFactor);
    m_isLeaf = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Tile::_Invalidate()
    {
    m_graphic = nullptr;
    m_contentRange = ElementAlignedBox3d();
    InitTolerance();
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
        double diagSq = range.low.DistanceSquared(range.high);
        if (diagSq >= minRangeDiagonalSquared || diagSq == 0.0) // ###TODO_ELEMENT_TILE: Dumb single-point primitives...
            return true;
        }

    // No damaged range is large enough to contribute to this tile, so no need to regenerate it.
    return false;
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
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
double Tile::_GetMaximumSize() const
    {
    // returning 0.0 signifies undisplayable tile...
    return m_displayable ? s_tileScreenSize * m_zoomFactor : 0.0;
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   02/17
//=======================================================================================
struct MeshGenerator
{
private:
    typedef bmap<MeshMergeKey, MeshBuilderPtr> BuilderMap;

    TileCR              m_tile;
    GeometryOptions     m_options;
    double              m_tolerance;
    double              m_vertexTolerance;
    double              m_facetAreaTolerance;
    BuilderMap          m_builderMap;
    DRange3d            m_tileRange;
    LoadContextCR       m_loadContext;
    size_t              m_geometryCount = 0;
    FeatureTable        m_featureTable;
    DRange3d            m_contentRange = DRange3d::NullRange();
    bool                m_maxGeometryCountExceeded = false;

    static constexpr double GetVertexClusterThresholdPixels() { return 5.0; }
    static constexpr size_t GetDecimatePolyfacePointCount() { return 100; }

    MeshBuilderR GetMeshBuilder(MeshMergeKey& key);
    DgnElementId GetElementId(GeometryR geom) const { return m_maxGeometryCountExceeded ? DgnElementId() : geom.GetEntityId(); }

    void AddPolyfaces(GeometryR geom, double rangePixels, bool isContained);
    void AddPolyfaces(PolyfaceList& polyfaces, GeometryR geom, double rangePixels, bool isContained);
    void AddPolyface(Polyface& polyfaces, GeometryR geom, double rangePixels, bool isContained);

    void AddStrokes(GeometryR geom, double rangePixels, bool isContained);
    void AddStrokes(StrokesList& strokes, GeometryR geom, double rangePixels, bool isContained);
    void AddStrokes(StrokesR strokes, GeometryR geom, double rangePixels, bool isContained);
    Strokes ClipStrokes(StrokesCR strokes) const;
public:
    MeshGenerator(TileCR tile, GeometryOptionsCR options, LoadContextCR loadContext);

    // Add meshes to the MeshBuilder map
    void AddMeshes(GeometryList const& geometries, bool doRangeTest);
    void AddMeshes(GeometryR geom, bool doRangeTest);
    void AddMeshes(GeomPartR part, bvector<GeometryCP> const& instances);

    // Return a list of all meshes currently in the builder map
    MeshList GetMeshes();
    // Return a tight bounding volume
    DRange3dCR GetContentRange() const { return m_contentRange; }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
MeshGenerator::MeshGenerator(TileCR tile, GeometryOptionsCR options, LoadContextCR loadContext)
  : m_tile(tile), m_options(options), m_tolerance(tile.GetTolerance()), m_vertexTolerance(m_tolerance*ToleranceRatio::Vertex()),
    m_facetAreaTolerance(m_tolerance*ToleranceRatio::FacetArea()), m_tileRange(tile.GetTileRange()), m_loadContext(loadContext),
    m_featureTable(nullptr != loadContext.GetRenderSystem() ? loadContext.GetRenderSystem()->_GetMaxFeaturesPerBatch() : s_hardMaxFeaturesPerTile)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
MeshBuilderR MeshGenerator::GetMeshBuilder(MeshMergeKey& key)
    {
    auto found = m_builderMap.find(key);
    if (m_builderMap.end() != found)
        return *found->second;

    bool is2d = m_tile.GetElementRoot().Is2d();
    MeshBuilderPtr builder = MeshBuilder::Create(*key.m_params, m_vertexTolerance, m_facetAreaTolerance, &m_featureTable, key.m_primitiveType, m_tileRange, is2d);
    m_builderMap[key] = builder;
    return *builder;
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
    double rangePixels = geomRange.DiagonalDistance() / m_tolerance;
    if (rangePixels < s_minRangeBoxSize && 0.0 < geomRange.DiagonalDistance()) // ###TODO_ELEMENT_TILE: single point primitives have an empty range...
        return;   // ###TODO: -- Produce an artifact from optimized bounding box to approximate from range.

    bool isContained = !doRangeTest || geomRange.IsContained(m_tileRange);
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
    double rangePixels = geomRange.DiagonalDistance() / m_tolerance;
    if (rangePixels < s_minRangeBoxSize)
        return;

    auto facetOptions = first->CreateFacetOptions(m_tolerance, m_options.m_normalMode);

    // Get the polyfaces and strokes with no transform applied
    PolyfaceList polyfaces = part.GetPolyfaces(*facetOptions, nullptr);
    facetOptions->SetNormalsRequired(false);
    StrokesList strokes = part.GetStrokes(*facetOptions, nullptr);

    // For each instance, transform the polyfaces and add them to the mesh
    Transform invTransform = Transform::FromIdentity();
    for (GeometryCP instance : instances)
        {
        Transform instanceTransform = Transform::FromProduct(instance->GetTransform(), invTransform);
        invTransform.InverseOf(instance->GetTransform());
        for (auto& polyface : polyfaces)
            {
            polyface.Transform(instanceTransform);
            AddPolyface(polyface, const_cast<GeometryR>(*instance), rangePixels, true);
            }

        for (auto& strokeList : strokes)
            {
            strokeList.Transform(instanceTransform);
            AddStrokes(strokeList, *const_cast<GeometryP>(instance), rangePixels, true);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddPolyfaces(GeometryR geom, double rangePixels, bool isContained)
    {
    auto polyfaces = geom.GetPolyfaces(m_tolerance, m_options.m_normalMode);
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
void MeshGenerator::AddPolyface(Polyface& tilePolyface, GeometryR geom, double rangePixels, bool isContained)
    {
    PolyfaceHeaderP polyface = tilePolyface.m_polyface.get();
    if (nullptr == polyface || 0 == polyface->GetPointCount())
        return;

    // NB: The polyface is shared amongst many instances, each of which may have its own display params. Use the params from the instance.
    DisplayParamsCR displayParams = geom.GetDisplayParams();
    DgnDbR db = m_tile.GetElementRoot().GetDgnDb();
    bool hasTexture = displayParams.IsTextured();

    MeshMergeKey key(displayParams, nullptr != polyface->GetNormalIndexCP(), Mesh::PrimitiveType::Mesh);
    MeshBuilderR builder = GetMeshBuilder(key);

    bool doDecimate = !m_tile.IsLeaf() && geom.DoDecimate() && polyface->GetPointCount() > GetDecimatePolyfacePointCount();
    bool doVertexCluster = !doDecimate && geom.DoVertexCluster() && rangePixels < GetVertexClusterThresholdPixels();

    if (doDecimate)
        polyface->DecimateByEdgeCollapse(m_tolerance, 0.0);

    bool                    anyContributed = false;
    uint32_t                fillColor = displayParams.GetFillColor();

    builder.BeginPolyface(*polyface, MeshEdgeCreationOptions(tilePolyface.m_displayEdges ? MeshEdgeCreationOptions::DefaultEdges : MeshEdgeCreationOptions::NoEdges));
    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*polyface); visitor->AdvanceToNextFace(); /**/)
        {
        if (isContained || m_tileRange.IntersectsWith(DRange3d::From(visitor->GetPointCP(), static_cast<int32_t>(visitor->Point().size()))))
            {
            anyContributed = true;
            DgnElementId elemId = GetElementId(geom);
            builder.AddTriangle(*visitor, displayParams.GetRenderingAsset(), db, featureFromParams(elemId, displayParams), doVertexCluster, hasTexture, fillColor);
            m_contentRange.Extend(visitor->Point());
            }
        }
    DRange3d        polyfaceRange = polyface->PointRange();
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
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
Strokes MeshGenerator::ClipStrokes(StrokesCR input) const
    {
    // Might be more efficient to modify input in-place.
    Strokes output(*input.m_displayParams, input.m_disjoint);
    enum    State { kInside, kOutside, kCrossedOutside };

    output.m_strokes.reserve(input.m_strokes.size());

    for (auto const& inputStroke : input.m_strokes)
        {
        auto const&     points = inputStroke.m_points;
        DRange3d        range = DRange3d::From(points);
        DPoint3d        rangeCenter = DPoint3d::FromInterpolate(range.low, .5, range.high);

        if (points.size() <= 1)
            continue;

        DPoint3d prevPt = points.front();
        State   prevState = m_tileRange.IsContained(prevPt) ? kInside : kOutside;
        if (kInside == prevState)
            {
            output.m_strokes.push_back(Strokes::PointList(inputStroke.m_startDistance, rangeCenter));
            output.m_strokes.back().m_points.push_back(prevPt);
            }

        double   length = inputStroke.m_startDistance;       // Cumulative length along polyline.
        for (size_t i = 1; i < points.size(); i++)
            {
            auto nextPt = points[i];
            bool contained = m_tileRange.IsContained(nextPt);
            State nextState = contained ? kInside : (kInside == prevState ? kCrossedOutside : kOutside);
            if (kOutside == nextState && kOutside == prevState)
                {
                // The endpoints of a segment may lie outside of the range, but intersect it...
                double unused1, unused2;
                DSegment3d unused3;
                DSegment3d segment = DSegment3d::From(prevPt, nextPt);
                if (m_tileRange.IntersectBounded(unused1, unused2, unused3, segment))
                    nextState = kCrossedOutside;
                }

            if (kOutside != nextState)
                {
                if (kOutside == prevState)
                    {
                    // back inside - start a new line string...
                    output.m_strokes.push_back(Strokes::PointList(length, rangeCenter));
                    output.m_strokes.back().m_points.push_back(prevPt);
                    }

                BeAssert(!output.m_strokes.empty());
                output.m_strokes.back().m_points.push_back(nextPt);
                }
            length += prevPt.Distance(nextPt);
            prevState = nextState;
            prevPt = nextPt;
            }

        BeAssert(output.m_strokes.empty() || 1 < output.m_strokes.back().m_points.size());
        }

    return output;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddStrokes(GeometryR geom, double rangePixels, bool isContained)
    {
    auto strokes = geom.GetStrokes(*geom.CreateFacetOptions(m_tolerance, NormalMode::Never));
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
        {
        Strokes clippedStrokes = ClipStrokes(strokes);
        AddStrokes(clippedStrokes, geom, rangePixels, true);
        return;
        }

    if (strokes.m_strokes.empty())
        return; // avoid potentially creating the builder below...

    // NB: The strokes are shared amongst many instances, each of which may have its own display params. Use the params from the instance.
    DisplayParamsCR displayParams = geom.GetDisplayParams();
    MeshMergeKey key(displayParams, false, strokes.m_disjoint ? Mesh::PrimitiveType::Point : Mesh::PrimitiveType::Polyline);
    MeshBuilderR builder = GetMeshBuilder(key);

    uint32_t fillColor = displayParams.GetLineColor();
    DgnElementId elemId = GetElementId(geom);
    for (auto& stroke : strokes.m_strokes)
        {
        m_contentRange.Extend(stroke.m_points);
        builder.AddPolyline(stroke.m_points, featureFromParams(elemId, displayParams), rangePixels < GetVertexClusterThresholdPixels(), fillColor, stroke.m_startDistance, stroke.m_rangeCenter);
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
            {
            mesh->Close();
            meshes.push_back(mesh);
            }
        }

    // Do not allow vertices outside of this tile's range to expand its content range
    clipContentRangeToTileRange(m_contentRange, m_tileRange);

    meshes.m_features = std::move(m_featureTable);
    return meshes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
MeshList Tile::GenerateMeshes(GeometryList const& geometries, bool doRangeTest, LoadContextCR loadContext) const
    {
    GeometryOptions options;
    MeshGenerator generator(*this, options, loadContext);
    generator.AddMeshes(geometries, doRangeTest);
    
    MeshList meshes;
    if (!loadContext.WasAborted())
        {
        meshes = generator.GetMeshes();
        m_contentRange = ElementAlignedBox3d(generator.GetContentRange());
        }

    return meshes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Render::Primitives::GeometryCollection Tile::GenerateGeometry(LoadContextCR context)
    {
    Render::Primitives::GeometryCollection geom;

    auto const& root = GetElementRoot();
    GeometryList geometries = CollectGeometry(context);
    auto collection = CreateGeometryCollection(geometries, context);
    if (collection.IsEmpty() && !geometries.empty())
        collection.MarkIncomplete();

    if (geometries.ContainsCurves())
        collection.MarkCurved();

    return collection;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Render::Primitives::GeometryCollection Tile::CreateGeometryCollection(GeometryList const& geometries, LoadContextCR context) const
    {
    GeometryOptions options;
    Render::Primitives::GeometryCollection collection;
    bmap<GeomPartCP, bvector<GeometryCP>> parts;
    MeshGenerator generator(*this, options, context);

    for (auto const& geom : geometries)
        {
        if (context.WasAborted())
            return collection;

        auto part = geom->GetPart();
        if (part.IsNull())
            {
            generator.AddMeshes(*geom, true);
            continue;
            }

        auto iter = parts.find(part.get());
        if (parts.end() == iter)
            iter = parts.Insert(part.get(), bvector<GeometryCP>()).first;

        iter->second.push_back(geom.get());
        }

    for (auto& kvp : parts)
        {
        if (context.WasAborted())
            break;

        generator.AddMeshes(*const_cast<GeomPartP>(kvp.first), kvp.second);
        }

    if (!context.WasAborted())
        {
        collection.Meshes() = generator.GetMeshes();
        m_contentRange = ElementAlignedBox3d(generator.GetContentRange());
        if (!geometries.IsComplete())
            collection.MarkIncomplete();
        }

    return collection;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d Tile::GetDgnRange() const
    {
    DRange3d range;
    GetRoot().GetLocation().Multiply(range, GetTileRange());
    return range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
double ElementCollector::ComputeLeafTolerance() const
    {
#if defined(TODO_ELEMENT_TILE)
    // This uses minimum zoom. It is far too small to use in the general case, and overkill for any geometry not modeled on tiny scales.
    return DgnUnits::OneMillimeter() / s_minToleranceRatio;
#else
    // This function is used when we decide to create a leaf node because the tile's range contains too few elements to be worth subdividing.
    // Computes the tile's tolerance based on the range of the smallest element within the range.
    double minSizeSq = 0.0;
    for (auto iter = m_entries.rbegin(); iter != m_entries.rend(); ++iter)
        {
        if (0.0 < (minSizeSq = iter->first))
            break;
        }

    if (0.0 >= minSizeSq)
        return 0.001;

    return sqrt(minSizeSq) / s_minToleranceRatio;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryList Tile::CollectGeometry(LoadContextCR loadContext)
    {
    GeometryList geometries;

    auto& root = GetElementRoot();
    auto model = root.GetModel();
    if (model.IsNull() || DgnDbStatus::Success != model->FillRangeIndex())
        return geometries;

    // Collect the set of largest elements for this tile's range, excluding any too small to contribute at this tile's tolerance.
    uint32_t maxFeatures = s_hardMaxFeaturesPerTile; // Note: Element != Feature - could have multiple features per element
    auto sys = loadContext.GetRenderSystem();
    if (nullptr != sys)
        maxFeatures = std::min(maxFeatures, sys->_GetMaxFeaturesPerBatch());

    double minRangeDiagonalSq = s_minRangeBoxSize * m_tolerance;
    minRangeDiagonalSq *= minRangeDiagonalSq;
    ElementCollector collector(GetDgnRange(), *model->GetRangeIndex(), minRangeDiagonalSq, loadContext, maxFeatures);

    if (loadContext.WasAborted())
        return geometries;

    if (collector.AnySkipped())
        {
        // We may want to turn this into a leaf node if it contains strictly linear geometry - but we can't do that if any elements were excluded
        geometries.MarkIncomplete();
        }

    if (collector.AnySkipped())
        geometries.MarkIncomplete();

    IFacetOptionsPtr facetOptions = Geometry::CreateFacetOptions(m_tolerance);

    facetOptions->SetHideSmoothEdgesWhenGeneratingNormals(false);        // We'll do this ourselves when generating meshes - This will turn on sheet edges that should be hidden (Pug.dgn).
    Transform transformFromDgn;
    transformFromDgn.InverseOf(root.GetLocation());

    // Process the geometry of each element selected for inclusion in this tile
    TileContext tileContext(geometries, root, GetDgnRange(), *facetOptions, transformFromDgn, m_tolerance, loadContext);
    for (auto const& entry : collector.GetEntries())
        {
        tileContext.ProcessElement(entry.second, entry.first);
        if (loadContext.WasAborted())
            {
            geometries.clear();
            break;
            }
        else if (tileContext.GetGeometryCount() >= maxFeatures)
            {
            BeAssert(!IsLeaf());
            tileContext.TruncateGeometryList(maxFeatures);
            break;
            }
        }

    if (!loadContext.WasAborted() && !IsLeaf() && !HasZoomFactor() && collector.GetEntries().size() <= s_minElementsPerTile)
        {
        // If no elements were skipped and only a small number of elements exist within this tile's range:
        //  - Make it a leaf tile, if it contains no curved geometry; otherwise
        //  - Mark it so that it will have only a single child tile, containing the same geometry faceted at a higher resolution
        // Note: element count is obviously a coarse heuristic as we have no idea the complexity of each element's geometry
        // Also note that if we're a child of a tile with zoom factor, we already have our own (higher) zoom factor
        if (!geometries.ContainsCurves())
            SetIsLeaf();
        else
            SetZoomFactor(1.0);
        }

    return geometries;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicPtr TileContext::FinishGraphic(GeometryAccumulatorR accum, TileBuilder& builder)
    {
    for (auto& geom : accum.GetGeometries())
        PushGeometry(*geom);

    m_root.AddCachedGeometry(std::move(accum.GetGeometries()), accum.GetElementId(), builder.GetRangeDiagonalSquared());

    return m_finishedGraphic; // carries no useful info and is just going to be discarded...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicPtr TileContext::FinishSubGraphic(GeometryAccumulatorR accum, TileSubGraphic& subGf)
    {
    DgnGeometryPartCR input = subGf.GetInput();
    if (accum.GetGeometries().empty())
        BeAssert(false);

    subGf.SetOutput(*GeomPart::Create(input.GetBoundingBox(), accum.GetGeometries()));
    if (accum.GetGeometries().empty())
        BeAssert(false);

    return m_finishedGraphic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileContext::ProcessElement(DgnElementId elemId, double rangeDiagonalSquared)
    {
    try
        {
        if (!m_root.GetCachedGeometry(m_geometries, elemId, rangeDiagonalSquared))
            {
            m_curElemId = elemId;
            m_curRangeDiagonalSquared = rangeDiagonalSquared;
            VisitElement(elemId, false);
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

#if defined (BENTLEYCONFIG_PARASOLID) 
// ###TODO: ugh.
static ThreadedLocalParasolidHandlerStorageMark s_tempParasolidThreadedHandlerStorageMark;
#endif


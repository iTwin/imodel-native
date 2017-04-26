/*--------------------------------------------------------------------------------------+                                                                                           
|
|     $Source: DgnCore/ElementTileTree.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/ElementTileTree.h>
#include <folly/BeFolly.h>
#include <DgnPlatform/RangeIndex.h>
#if defined (BENTLEYCONFIG_PARASOLID) 
#include <DgnPlatform/DgnBRep/PSolidUtil.h>
#endif

#if defined(NDEBUG)
#define ELEMENT_TILE_DEBUG_RANGE false
#else
#define ELEMENT_TILE_DEBUG_RANGE true
#endif

USING_NAMESPACE_ELEMENT_TILETREE
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES

BEGIN_UNNAMED_NAMESPACE

#if defined (BENTLEYCONFIG_PARASOLID) 

// The ThreadLocalParasolidHandlerStorageMark sets up the local storage that will be used 
// by all threads.

typedef RefCountedPtr <struct ThreadedParasolidErrorHandlerInnerMark>     ThreadedParasolidErrorHandlerInnerMarkPtr;


class   ParasolidException {};

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
constexpr size_t s_maxGeometryIdCount = 0xffff;  // Max batch table ID - 16-bit unsigned integers
constexpr double s_tileScreenSize = 512.0;
constexpr double s_minToleranceRatio = s_tileScreenSize;
constexpr uint32_t s_minElementsPerTile = 50;
constexpr double s_minLeafTolerance = 0.001;
constexpr double s_solidPrimitivePartCompareTolerance = 1.0E-5;
constexpr double s_spatialRangeMultiplier = 4.0;

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
struct RangeAccumulator : RangeIndex::Traverser
{
    DRange3dR       m_range;
    bool            m_is2d;

    RangeAccumulator(DRange3dR range, bool is2d) : m_range(range), m_is2d(is2d) { m_range = DRange3d::NullRange(); }

    bool _AbortOnWriteRequest() const override { return true; }
    Accept _CheckRangeTreeNode(RangeIndex::FBoxCR, bool) const override { return Accept::Yes; }
    Stop _VisitRangeTreeEntry(RangeIndex::EntryCR entry) override
        {
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

struct GeometryProcessorContext;

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct TileGeometryProcessor : IGeometryProcessor
{
    //=======================================================================================
    // @bsistruct                                                   Paul.Connelly   12/16
    //=======================================================================================
    enum class Result { Success, NoGeometry, Aborted };
private:
    IFacetOptionsR              m_facetOptions;
    IFacetOptionsPtr            m_targetFacetOptions;
    mutable IFacetOptionsPtr    m_lsStrokerOptions;
    RootR                       m_root;
    GeometryList&               m_geometries;
    DRange3d                    m_range;
    DRange3d                    m_tileRange;
    GeometryListBuilder         m_geometryListBuilder;
    double                      m_minRangeDiagonal;
    double                      m_minTextBoxSize;
    double                      m_tolerance;
    DgnElementIdSet             m_excludedElements;
    bool                        m_is2d;
    bool                        m_wantCacheSolidPrimitives = false;
protected:
//#define ELEMENT_TILE_TRUNCATE_PLANAR
#if defined(ELEMENT_TILE_TRUNCATE_PLANAR)
    bool                        m_anyCurvedGeometry = false;
#else
    bool                        m_anyCurvedGeometry = true;
#endif

    virtual bool _AcceptGeometry(GeometryCR geom, DgnElementId elemId);
    virtual bool _AcceptElement(DRange3dCR range, DgnElementId elemId);

    void PushGeometry(GeometryR geom);
    void AddElementGeometry(GeometryR geom);

    IFacetOptionsP _GetFacetOptionsP() override { return &m_facetOptions; }

    bool _ProcessCurveVector(CurveVectorCR curves, bool filled, SimplifyGraphic& gf) override;
    bool _ProcessSolidPrimitive(ISolidPrimitiveCR prim, SimplifyGraphic& gf) override;
    bool _ProcessSurface(MSBsplineSurfaceCR surface, SimplifyGraphic& gf) override;
    bool _ProcessPolyface(PolyfaceQueryCR polyface, bool filled, SimplifyGraphic& gf) override;
    bool _ProcessBody(IBRepEntityCR solid, SimplifyGraphic& gf) override;
    bool _ProcessTextString(TextStringCR, SimplifyGraphic&) override;

    double _AdjustZDepth(double zDepthIn) override
        {
        return Target::DepthFromDisplayPriority(static_cast<int32_t>(zDepthIn));
        }

    UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR, SimplifyGraphic&) const override { return UnhandledPreference::Facet; }
    UnhandledPreference _GetUnhandledPreference(CurveVectorCR, SimplifyGraphic&)     const override { return UnhandledPreference::Facet; }
    UnhandledPreference _GetUnhandledPreference(IBRepEntityCR, SimplifyGraphic&)     const override { return UnhandledPreference::Facet; }

    TileGeometryProcessor(GeometryList& geoms, RootR root, DRange3dCR range, IFacetOptionsR facetOptions, TransformCR transformFromDgn, double tileTolerance, bool surfacesOnly, bool is2d, double rangeTolerance);
public:
    TileGeometryProcessor(GeometryList& geometries, RootR root, DRange3dCR range, IFacetOptionsR facetOptions, TransformCR transformFromDgn, double tolerance, bool surfacesOnly, bool is2d)
        : TileGeometryProcessor(geometries, root, range, facetOptions, transformFromDgn, tolerance, surfacesOnly, is2d, tolerance) { }

    void ProcessElement(ViewContextR context, DgnElementId elementId, DRange3dCR range);
    Result OutputGraphics(GeometryProcessorContext& context);
    void AddGeomPart (Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, GeometryParamsR geomParams, GraphicParamsR graphicParams, ViewContextR viewContext);


    DgnDbR GetDgnDb() const { return m_root.GetDgnDb(); }
    RootR GetRoot() const { return m_root; }

    bool BelowMinRange(DRange3dCR range) const
        {
        // Avoid processing any elements with range smaller than roughly half a pixel...
        auto diag = range.DiagonalDistance();
        return diag < m_minRangeDiagonal && 0.0 < diag; // ###TODO_ELEMENT_TILE: Dumb single-point primitives...
        }

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

    bool _DoLineStyleStroke(Render::LineStyleSymbCR lsSymb, IFacetOptionsPtr& opts, SimplifyGraphic& gf) const override
        {
        opts = GetLineStyleStrokerOptions(lsSymb);
        return opts.IsValid();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Ray.Bentley     11/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    UnhandledPreference _GetUnhandledPreference(TextStringCR textString, SimplifyGraphic& simplifyGraphic) const override 
        {
        DRange2d        range = textString.GetRange();
        Transform       transformToTile = Transform::FromProduct(GetTransformFromDgn(), simplifyGraphic.GetLocalToWorldTransform(), textString.ComputeTransform());
        double          minTileDimension = transformToTile.ColumnXMagnitude() * std::min(range.XLength(), range.YLength());

        return minTileDimension < m_minTextBoxSize ? UnhandledPreference::Box : UnhandledPreference::Curve;
        }

    bool WantSurfacesOnly() const { return m_geometryListBuilder.WantSurfacesOnly(); }
    DgnElementId GetCurrentElementId() const { return m_geometryListBuilder.GetElementId(); }
    TransformCR GetTransformFromDgn() const { return m_geometryListBuilder.GetTransform(); }
    DisplayParamsCR CreateDisplayParams(SimplifyGraphic& gf, bool ignoreLighting) const { return m_geometryListBuilder.GetDisplayParamsCache().Get(gf.GetCurrentGraphicParams(), gf.GetCurrentGeometryParams(), ignoreLighting); }
    DisplayParamsCR CreateDefaultDisplayParams(SimplifyGraphic& gf) const { return CreateDisplayParams(gf, m_is2d); }

    bool ContainsCurvedGeometry() const { return m_anyCurvedGeometry; }
    bool AnyElementsExcluded() const { return !m_excludedElements.empty(); }
    DgnElementIdSet const& GetExcludedElements() const { return m_excludedElements; }
};

/*---------------------------------------------------------------------------------**//**
* After TileGeometryProcessor processes elements within tile range and large enough
* to contribute to tile mesh, we may determine that the mesh consists entirely of uncurved
* geometry. We want to treat it as a leaf node as there is no point in subdividing
* a tile containing only uncurved stuff. But we need to add in any geometry which was
* excluded previously for being too small.
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
struct ExcludedGeometryProcessor : TileGeometryProcessor
{
private:
    virtual bool _AcceptElement(DRange3dCR range, DgnElementId elemId) override { return true; }
    virtual bool _AcceptGeometry(GeometryCR geom, DgnElementId elemId) override
        {
        if (!BelowMinRange(geom.GetTileRange()))
            return false;

        m_anyCurvedGeometry = geom.IsCurved();
        return true;
        }
public:
    ExcludedGeometryProcessor(GeometryList& geometries, RootR root, DRange3dCR range, IFacetOptionsR facetOptions, TransformCR transformFromDgn, double leafTolerance, bool surfacesOnly, bool is2d, double rangeTolerance) : TileGeometryProcessor(geometries, root, range, facetOptions, transformFromDgn, leafTolerance, surfacesOnly, is2d, rangeTolerance)
    {
    //
    }

    void ProcessElements(ViewContextR context, DgnElementIdSet const& elemIds)
        {
        DRange3d unusedElementRange;
        for (auto const& elemId : elemIds)
            {
            ProcessElement(context, elemId, unusedElementRange);
            if (ContainsCurvedGeometry())
                break; // turns out we do have curved geometry - don't make a leaf tile
            }
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryProcessor::TileGeometryProcessor(GeometryList& geometries, RootR root, DRange3dCR range, IFacetOptionsR facetOptions, TransformCR transformFromDgn, double tileTolerance, bool surfacesOnly, bool is2d, double rangeTolerance)
  : m_geometries (geometries), m_facetOptions(facetOptions), m_targetFacetOptions(facetOptions.Clone()), m_root(root), m_range(range), m_geometryListBuilder(root.GetDgnDb(), transformFromDgn, surfacesOnly), 
    m_tolerance(tileTolerance), m_is2d(is2d)
    {
    static const double s_minTextBoxSize = 1.0;     // Below this ratio to tolerance  text is rendered as box.

    m_targetFacetOptions->SetChordTolerance(facetOptions.GetChordTolerance() * transformFromDgn.ColumnXMagnitude());
    m_minRangeDiagonal = s_minRangeBoxSize * rangeTolerance;
    m_minTextBoxSize  = s_minTextBoxSize * rangeTolerance;
    GetTransformFromDgn().Multiply (m_tileRange, m_range);
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
struct GeometryProcessorContext : NullContext
{
DEFINE_T_SUPER(NullContext);

private:
    TileGeometryProcessor*          m_processor;
    BeSQLite::CachedStatementPtr    m_statement;
    LoadContextCR                   m_loadContext;

    bool IsValueNull(int index) { return m_statement->IsColumnNull(index); }

    Render::GraphicBuilderPtr _CreateGraphic(Render::GraphicBuilder::CreateParams const& params) override
        {
        return new SimplifyGraphic(params, *m_processor, *this);
        }

    StatusInt _VisitElement(DgnElementId elementId, bool allowLoad) override;
    Render::GraphicPtr _StrokeGeometry(GeometrySourceCR, double) override;
    void _AddSubGraphic(Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, GeometryParamsR geomParams) override;
    bool _CheckStop() override { return WasAborted() || AddAbortTest(m_loadContext.WasAborted()); }

    Render::MaterialPtr _GetMaterial(DgnMaterialId id) const override
        {
        Render::SystemP system = m_processor->GetRoot().GetRenderSystem();
        return nullptr != system ? system->_GetMaterial(id, m_processor->GetDgnDb()) : nullptr;
        }

    static Render::ViewFlags GetDefaultViewFlags();
public:
    GeometryProcessorContext(TileGeometryProcessor& processor, RootR root, LoadContextCR loadContext)
        : m_processor(&processor), m_statement(root.GetDgnDb().GetCachedStatement(root.Is3d() ? GeometrySelector3d::GetSql() : GeometrySelector2d::GetSql())), m_loadContext(loadContext)
        {
        SetDgnDb(root.GetDgnDb());
        m_is3dView = root.Is3d(); // force Brien to call _AddArc2d() if we're in a 2d model...
        SetViewFlags(GetDefaultViewFlags());
        }

    void SetProcessor(TileGeometryProcessor& proc) { m_processor = &proc; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
struct GeometryCollector : RangeIndex::Traverser
{
    TileGeometryProcessor&      m_processor;
    GeometryProcessorContext&   m_context;
    RangeIndex::FBox            m_range;

    GeometryCollector(DRange3dCR range, TileGeometryProcessor& proc, GeometryProcessorContext& context)
        : m_range(range), m_processor(proc), m_context(context) { }

    Accept _CheckRangeTreeNode(RangeIndex::FBoxCR box, bool is3d) const override
        {
        return !m_context.CheckStop() && box.IntersectsWith(m_range) ? Accept::Yes : Accept::No;
        }

    Stop _VisitRangeTreeEntry(RangeIndex::EntryCR entry) override
        {
        if (m_context.CheckStop())
            return Stop::Yes;

        if (entry.m_range.IntersectsWith(m_range))
            m_processor.ProcessElement(m_context, entry.m_id, entry.m_range.ToRange3d());

        return Stop::No;
        }

    TileGeometryProcessor::Result Collect()
        {
        auto model = m_processor.GetRoot().GetModel();
        if (model.IsNull() || DgnDbStatus::Success != model->FillRangeIndex())
            return TileGeometryProcessor::Result::NoGeometry;

        return Stop::Yes == model->GetRangeIndex()->Traverse(*this) ? TileGeometryProcessor::Result::Aborted : TileGeometryProcessor::Result::Success;
        }
};

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Loader::Loader(TileR tile, TileTree::TileLoadStatePtr loads)
    : T_Super("", tile, loads, "")
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
    auto& system = *root.GetRenderSystem();

    GeometryOptions options;
    LoadContext loadContext(this);
    auto geometry = tile.GenerateGeometry(options, loadContext);

    if (loadContext.WasAborted())
        return ERROR;

    PolylineArgs polylineArgs;
    MeshArgs meshArgs;
    bvector<Render::GraphicPtr> graphics;
    auto& geometryMeshes = geometry.Meshes();
    for (auto const& mesh : geometryMeshes)
        {
        bool haveMesh = !mesh->Triangles().empty();
        bool havePolyline = !haveMesh && !mesh->Polylines().empty();
        if (!haveMesh && !havePolyline)
            continue;

        Render::GraphicPtr thisGraphic;
        if (haveMesh)
            {
            if (meshArgs.Init(*mesh, system, root.GetDgnDb()))
                thisGraphic = system._CreateTriMesh(meshArgs, root.GetDgnDb(), mesh->GetDisplayParams().GetGraphicParams());
            }
        else
            {
            BeAssert(havePolyline);
            if (polylineArgs.Init(*mesh))
                thisGraphic = system._CreateIndexedPolylines(polylineArgs, root.GetDgnDb(), mesh->GetDisplayParams().GetGraphicParams());
            }

        if (thisGraphic.IsValid())
            graphics.push_back(thisGraphic);
        }

    if (!graphics.empty())
        {
        if (root.WantDebugRanges())
            {
            ColorDef color = geometry.IsEmpty() ? ColorDef::Red() : tile.IsLeaf() ? ColorDef::DarkBlue() : ColorDef::DarkOrange();
            GraphicParams gfParams;
            gfParams.SetLineColor(color);
            gfParams.SetFillColor(ColorDef::Green());
            gfParams.SetWidth(0);
            gfParams.SetLinePixels(tile.IsLeaf() ? GraphicParams::LinePixels::Code5 : GraphicParams::LinePixels::Code4);

            Render::GraphicBuilderPtr rangeGraphic = system._CreateGraphic(GraphicBuilder::CreateParams(root.GetDgnDb()));
            rangeGraphic->ActivateGraphicParams(gfParams);
            rangeGraphic->AddRangeBox(tile.GetRange());
            graphics.push_back(rangeGraphic->Finish());
            }

        GraphicPtr graphic;
        switch (graphics.size())
            {
            case 0:
                break;
            case 1:
                graphic = *graphics.begin();
                break;
            default:
                graphic = system._CreateGraphicList(std::move(graphics), root.GetDgnDb());
                break;
            }

        if (graphic.IsValid())
            tile.SetGraphic(*system._CreateBatch(*graphic, std::move(geometryMeshes.m_features)));
        }

    // No point subdividing empty nodes - improves performance if we don't
    if (geometry.IsEmpty())
        tile.SetIsLeaf();   // ###TODO: Is this true - or can all the geometry be too small for this tile's tolerance?

    tile.SetIsReady();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Root::Root(GeometricModelR model, TransformCR transform, Render::SystemR system)
    : T_Super(model.GetDgnDb(), transform, "", &system), m_modelId(model.GetModelId()), m_name(model.GetName()),
    m_leafTolerance(s_minLeafTolerance), m_is3d(model.Is3dModel()), m_debugRanges(ELEMENT_TILE_DEBUG_RANGE), m_cacheGeometry(m_is3d)
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
    if (model.Is3dModel())
        {
        range = model.GetDgnDb().GeoLocation().GetProjectExtents();

        // This drastically reduces the time required to generate the root tile, so that the user sees *something* on the screen much sooner.
        range.ScaleAboutCenter(range, s_spatialRangeMultiplier);
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
        }

    // Translate world coordinates to center of range in order to reduce precision errors
    DPoint3d centroid = DPoint3d::FromInterpolate(range.low, 0.5, range.high);
    Transform transform = Transform::From(centroid);

    // ###TODO parasolid...
#if defined (BENTLEYCONFIG_PARASOLID) 
    ThreadedLocalParasolidHandlerStorageMark  parasolidParasolidHandlerStorageMark;
    PSolidKernelManager::StartSession();
#endif

    // Prepare for multi-threaded access to this stuff when generating geometry...
    T_HOST.GetFontAdmin().EnsureInitialized();
    model.GetDgnDb().Fonts().Update();

    RootPtr root = new Root(model, transform, system);
    Transform rangeTransform;

    rangeTransform.InverseOf(transform);
    DRange3d tileRange;
    rangeTransform.Multiply(tileRange, range);
    return root->LoadRootTile(tileRange, model) ? root : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Root::LoadRootTile(DRange3dCR range, GeometricModelR model)
    {
    m_rootTile = Tile::Create(*this, range);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
GeomPartPtr Root::GetGeomPart(DgnGeometryPartId partId) const
    {
    BeMutexHolder lock(m_mutex);
    auto iter = m_geomParts.find(partId);
    return iter != m_geomParts.end() ? iter->second.get() : nullptr;
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
    geomList.push_back(Geometry::Create(*geom, Transform::FromIdentity(), range, elemId, displayParams, prim.HasCurvedFaceOrEdge(), db));
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
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::AddGeomPart(DgnGeometryPartId partId, GeomPartR geomPart) const
    {
    BeMutexHolder lock(m_mutex);
    m_geomParts.Insert(partId, &geomPart);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool Root::WantCacheGeometry(DRange3dCR range) const
    {
    if (!m_cacheGeometry)
        return false;

    // Only cache geometry which occupies a significant portion of the model's range, since it will appear in many tiles
    constexpr double rangeRatio = 0.25;
    double diag = ComputeRange().DiagonalDistance();
    if (0.0 == diag)
        return false;

    BeAssert(m_is3d); // we only bother caching for 3d...want rangeRatio relative to actual range, not expanded range
    diag /= s_spatialRangeMultiplier;
    return range.DiagonalDistance() / diag >= rangeRatio;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::AddCachedGeometry(GeometryList&& geometry, DgnElementId elementId, DRange3dCR range) const
    {
    if (!WantCacheGeometry(range))
        return;

    for (auto& geom : geometry)
        geom->SetInCache(true);

    BeMutexHolder lock(m_mutex);
    m_geomLists.Insert(elementId, std::move(geometry));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool Root::GetCachedGeometry(GeometryList& geometry, DgnElementId elementId, DRange3dCR range) const
    {
    // NB: Check the range so that we don't acquire the mutex for geometry too small to be in cache
    if (!WantCacheGeometry(range))
        return false;

    BeMutexHolder lock(m_mutex);
    auto iter = m_geomLists.find(elementId);
    if (m_geomLists.end() == iter)
        return false;

    if (geometry.empty())
        geometry = iter->second;
    else
        geometry.insert(geometry.end(), iter->second.begin(), iter->second.end());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::Tile(Root& octRoot, TileTree::OctTree::TileId id, Tile const* parent, DRange3dCP range)
    : T_Super(octRoot, id, parent, false)
    {
    if (nullptr != parent)
        m_range = ElementAlignedBox3d(parent->ComputeChildRange(*this));
    else
        m_range.Extend(*range);

    double leafTolerance = GetElementRoot().GetLeafTolerance();
    double tileTolerance = m_range.DiagonalDistance() / s_minToleranceRatio;

    bool isLeaf = tileTolerance <= leafTolerance;
    if (isLeaf)
        {
        m_tolerance = leafTolerance;
        SetIsLeaf();
        }
    else
        {
        m_tolerance = tileTolerance;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Tile::IsElementCountLessThan(uint32_t threshold, double tolerance) const
    {
    struct Traverser : RangeIndex::Traverser
        {
        RangeIndex::FBox        m_range;
        uint32_t                m_threshold;
        uint32_t                m_count = 0;
        double                  m_minRangeDiagonal;

        Traverser(DRange3dCR range, uint32_t threshold, double tolerance) : m_range(range), m_threshold(threshold), m_minRangeDiagonal(s_minRangeBoxSize * tolerance) { }

        bool ThresholdReached() const { return m_count >= m_threshold; }

        Accept _CheckRangeTreeNode(RangeIndex::FBoxCR box, bool is3d) const override
            {
            return !ThresholdReached() && box.IntersectsWith(m_range) ? Accept::Yes : Accept::No;
            }

        Stop _VisitRangeTreeEntry(RangeIndex::EntryCR entry) override
            {
            if (!entry.m_range.IntersectsWith(m_range))
                return Stop::No;

            auto entryRange = entry.m_range.ToRange3d();
            if (entryRange.DiagonalDistance() >= m_minRangeDiagonal)
                ++m_count;

            return ThresholdReached() ? Stop::Yes : Stop::No;
            }
        };

    auto model = GetElementRoot().GetModel();
    auto index = model.IsValid() && DgnDbStatus::Success == model->FillRangeIndex() ? model->GetRangeIndex() : nullptr;
    if (nullptr == index)
        return true;    // no model => no elements...

    Traverser traverser(GetDgnRange(), threshold, tolerance);
    index->Traverse(traverser);
    return !traverser.ThresholdReached();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::TileLoaderPtr Tile::_CreateTileLoader(TileTree::TileLoadStatePtr loads)
    {
    return Loader::Create(*this, loads);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::TilePtr Tile::_CreateChild(TileTree::OctTree::TileId childId) const
    {
    return Tile::Create(const_cast<RootR>(GetElementRoot()), childId, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
double Tile::_GetMaximumSize() const
    {
    return s_tileScreenSize; // ###TODO: come up with a decent value, and account for device ppi
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   02/17
//=======================================================================================
struct MeshGenerator
{
private:
    typedef bmap<MeshMergeKey, MeshBuilderPtr> BuilderMap;

    TileCR          m_tile;
    GeometryOptions m_options;
    double          m_tolerance;
    double          m_vertexTolerance;
    double          m_facetAreaTolerance;
    BuilderMap      m_builderMap;
    DRange3d        m_tileRange;
    LoadContextCR   m_loadContext;
    size_t          m_geometryCount = 0;
    FeatureTable    m_featureTable;
    bool            m_maxGeometryCountExceeded = false;

    static constexpr double GetVertexClusterThresholdPixels() { return 5.0; }
    static constexpr size_t GetDecimatePolyfacePointCount() { return 100; }

    MeshBuilderR GetMeshBuilder(MeshMergeKey& key);
    DgnElementId GetElementId(GeometryR geom) const { return m_maxGeometryCountExceeded ? DgnElementId() : geom.GetEntityId(); }

    void AddPolyfaces(GeometryR geom, double rangePixels, bool isContained);
    void AddPolyfaces(PolyfaceList& polyfaces, GeometryR geom, double rangePixels, bool isContained);
    void AddPolyface(Polyface& polyfaces, GeometryR geom, double rangePixels, bool isContained);

    void AddStrokes(GeometryR geom, double rangePixels);
    void AddStrokes(StrokesList& strokes, GeometryR geom, double rangePixels);
    void AddStrokes(StrokesR strokes, GeometryR geom, double rangePixels);
public:
    MeshGenerator(TileCR tile, GeometryOptionsCR options, LoadContextCR loadContext);

    // Add meshes to the MeshBuilder map
    void AddMeshes(GeometryList const& geometries, bool doRangeTest);
    void AddMeshes(GeometryR geom, bool doRangeTest);
    void AddMeshes(GeomPartR part, bvector<GeometryCP> const& instances);

    // Return a list of all meshes currently in the builder map
    MeshList GetMeshes();
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
MeshGenerator::MeshGenerator(TileCR tile, GeometryOptionsCR options, LoadContextCR loadContext)
  : m_tile(tile), m_options(options), m_tolerance(tile.GetTolerance()), m_vertexTolerance(m_tolerance*ToleranceRatio::Vertex()),
    m_facetAreaTolerance(m_tolerance*ToleranceRatio::FacetArea()), m_tileRange(tile.GetTileRange()), m_loadContext(loadContext)
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

    MeshBuilderPtr builder = MeshBuilder::Create(*key.m_params, m_vertexTolerance, m_facetAreaTolerance, &m_featureTable, key.m_primitiveType);
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
        m_maxGeometryCountExceeded = (++m_geometryCount > s_maxGeometryIdCount);

    AddPolyfaces(geom, rangePixels, isContained);
    if (!m_options.WantSurfacesOnly())
        AddStrokes(geom, rangePixels);
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
            AddStrokes(strokeList, *const_cast<GeometryP>(instance), rangePixels);
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
    bool hasTexture = displayParams.HasTexture(db);

    MeshMergeKey key(displayParams, nullptr != polyface->GetNormalIndexCP(), Mesh::PrimitiveType::Mesh);
    MeshBuilderR builder = GetMeshBuilder(key);

    bool doDecimate = !m_tile.IsLeaf() && geom.DoDecimate() && polyface->GetPointCount() > GetDecimatePolyfacePointCount();
    bool doVertexCluster = !doDecimate && geom.DoVertexCluster() && rangePixels < GetVertexClusterThresholdPixels();

    if (doDecimate)
        polyface->DecimateByEdgeCollapse(m_tolerance, 0.0);

    bool anyContributed = false;
    uint32_t fillColor = displayParams.GetFillColor();
    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*polyface); visitor->AdvanceToNextFace(); /**/)
        {
        if (isContained || m_tileRange.IntersectsWith(DRange3d::From(visitor->GetPointCP(), static_cast<int32_t>(visitor->Point().size()))))
            {
            anyContributed = true;
            DgnElementId elemId = GetElementId(geom);
            builder.AddTriangle(*visitor, displayParams.GetMaterialId(), db, featureFromParams(elemId, displayParams), doVertexCluster, m_options.WantTwoSidedTriangles(), hasTexture, fillColor);
            }
        }

    // NB: The mesh's display params contain a fill color, which is used by the tri mesh primitive if the color table is empty (uniform)
    // But each polyface's display params may have a different fill color.
    // If a polyface contributes no vertices, we may end up incorrectly using its fill color for the primitive
    // Make sure the mesh's display params match one (any) mesh which actually contributed vertices, so that if the result is a uniform color,
    // we will use the fill color of the (only) mesh which contributed.
    if (anyContributed)
        builder.SetDisplayParams(displayParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddStrokes(GeometryR geom, double rangePixels)
    {
    auto strokes = geom.GetStrokes(*geom.CreateFacetOptions(m_tolerance, NormalMode::Never));
    AddStrokes(strokes, geom, rangePixels);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddStrokes(StrokesList& strokes, GeometryR geom, double rangePixels)
    {
    for (auto& stroke : strokes)
        AddStrokes(stroke, geom, rangePixels);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddStrokes(StrokesR strokes, GeometryR geom, double rangePixels)
    {
    if (m_loadContext.WasAborted())
        return;

    // NB: The polyface is shared amongst many instances, each of which may have its own display params. Use the params from the instance.
    DisplayParamsCR displayParams = geom.GetDisplayParams();
    MeshMergeKey key(displayParams, false, strokes.m_disjoint ? Mesh::PrimitiveType::Point : Mesh::PrimitiveType::Polyline);
    MeshBuilderR builder = GetMeshBuilder(key);

    uint32_t fillColor = displayParams.GetFillColor();
    DgnElementId elemId = GetElementId(geom);
    for (auto& strokePoints : strokes.m_strokes)
        builder.AddPolyline(strokePoints, featureFromParams(elemId, displayParams), rangePixels < GetVertexClusterThresholdPixels(), fillColor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
MeshList MeshGenerator::GetMeshes()
    {
    MeshList meshes;
    for (auto& builder : m_builderMap)
        if (!builder.second->GetMesh()->IsEmpty())
            meshes.push_back(builder.second->GetMesh());

    meshes.m_features = std::move(m_featureTable);
    return meshes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
MeshList Tile::GenerateMeshes(GeometryOptionsCR options, GeometryList const& geometries, bool doRangeTest, LoadContextCR loadContext) const
    {
    MeshGenerator generator(*this, options, loadContext);
    generator.AddMeshes(geometries, doRangeTest);
    return loadContext.WasAborted() ? MeshList() : generator.GetMeshes();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Render::Primitives::GeometryCollection Tile::GenerateGeometry(GeometryOptionsCR options, LoadContextCR context)
    {
    Render::Primitives::GeometryCollection geom;

    auto const& root = GetElementRoot();
    GeometryList geometries = CollectGeometry(m_tolerance, options.WantSurfacesOnly(), context);
    return CreateGeometryCollection(geometries, options, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Render::Primitives::GeometryCollection Tile::CreateGeometryCollection(GeometryList const& geometries, GeometryOptionsCR options, LoadContextCR context) const
    {
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
        collection.Meshes() = generator.GetMeshes();

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
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryList Tile::CollectGeometry(double tolerance, bool surfacesOnly, LoadContextCR loadContext)
    {
    auto& root = GetElementRoot();
    auto is2d = root.Is2d();
    IFacetOptionsPtr facetOptions = Geometry::CreateFacetOptions(tolerance);

    Transform transformFromDgn;
    transformFromDgn.InverseOf(root.GetLocation());

    GeometryList geometries;

    if (loadContext.WasAborted())
        return geometries;

    TileGeometryProcessor processor(geometries, root, GetDgnRange(), *facetOptions, transformFromDgn, tolerance, surfacesOnly, is2d);

    GeometryProcessorContext context(processor, root, loadContext);
    if (TileGeometryProcessor::Result::Success != processor.OutputGraphics(context) || processor.ContainsCurvedGeometry() || m_isLeaf)
        return geometries;

    // This tile contains no curved geometry, and it is not a leaf.
    // Add in geometry from any elements which were considered too small to contribute.
    // If none of that geometry is curved either, there's no point subdividing this tile - convert it to a leaf
    size_t numGeoms = geometries.size();
    auto leafTolerance = root.GetLeafTolerance();
    IFacetOptionsPtr leafFacetOptions = Geometry::CreateFacetOptions(leafTolerance);
    ExcludedGeometryProcessor reprocessor(geometries, root, GetDgnRange(), *leafFacetOptions, transformFromDgn, leafTolerance, surfacesOnly, is2d, tolerance);

    context.SetProcessor(reprocessor);
    reprocessor.ProcessElements(context, processor.GetExcludedElements());
    if (reprocessor.ContainsCurvedGeometry())
        {
        // ProcessElements() returns as soon it encounters curved geometry...remove anything it added before that.
        geometries.erase(geometries.begin() + numGeoms, geometries.end());
        }
    else
        {
        m_tolerance = root.GetLeafTolerance();
        SetIsLeaf();
        }

    return geometries;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometryProcessor::ProcessElement(ViewContextR context, DgnElementId elemId, DRange3dCR dgnRange)
    {
    try
        {
        if (!_AcceptElement(dgnRange, elemId))
            return;

        m_geometryListBuilder.Clear();
        bool haveCached = m_root.GetCachedGeometry(m_geometryListBuilder.GetGeometries(), elemId, dgnRange);
        if (!haveCached)
            {
            m_geometryListBuilder.SetElementId(elemId);
            context.VisitElement(elemId, false);
            }

        for (auto& geom : m_geometryListBuilder.GetGeometries())
            PushGeometry(*geom);

        if (!haveCached)
            m_root.AddCachedGeometry(std::move(m_geometryListBuilder.GetGeometries()), elemId, dgnRange);
        }
    catch (...)
        {
        // This shouldn't be necessary - but an uncaught exception will cause the processing to continue forever. (OpenCascade error in LargeHatchPlant.)
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessCurveVector(CurveVectorCR curves, bool filled, SimplifyGraphic& gf)
    {
    if (WantSurfacesOnly() && !curves.IsAnyRegionType())
        return true;

    bool isCurved = curves.ContainsNonLinearPrimitive();
    if (curves.IsAnyRegionType() && !isCurved)
        return false;   // process as facets.

    return m_geometryListBuilder.AddCurveVector(curves, filled, CreateDefaultDisplayParams(gf), gf.GetLocalToWorldTransform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessSolidPrimitive(ISolidPrimitiveCR prim, SimplifyGraphic& gf) 
    {
    DisplayParamsCR displayParams = CreateDefaultDisplayParams(gf);

    // ###TODO: Determine if the synchronization overhead of caching solid primitives is worth it.
    // Appears not - we spend significant time in IsSameStructureAndGeometry()
    if (m_wantCacheSolidPrimitives)
        {
        DRange3d range, thisTileRange;
        Transform tf = Transform::FromProduct(GetTransformFromDgn(), gf.GetLocalToWorldTransform());
        prim.GetRange(range);
        tf.Multiply(thisTileRange, range);
        if (thisTileRange.IsContained(m_tileRange))
            {
            ISolidPrimitivePtr clone = prim.Clone();
            GeomPartPtr geomPart = m_root.FindOrInsertGeomPart(*clone, range, displayParams, GetCurrentElementId());
            AddElementGeometry(*Geometry::Create(*geomPart, tf, thisTileRange, GetCurrentElementId(), displayParams, GetDgnDb()));
            return true;
            }
        }

    return m_geometryListBuilder.AddSolidPrimitive(prim, displayParams, gf.GetLocalToWorldTransform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessSurface(MSBsplineSurfaceCR surface, SimplifyGraphic& gf) 
    {
    return m_geometryListBuilder.AddSurface(surface, CreateDefaultDisplayParams(gf), gf.GetLocalToWorldTransform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessPolyface(PolyfaceQueryCR polyface, bool filled, SimplifyGraphic& gf) 
    {
    return m_geometryListBuilder.AddPolyface(polyface, filled, CreateDefaultDisplayParams(gf), gf.GetLocalToWorldTransform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessBody(IBRepEntityCR solid, SimplifyGraphic& gf) 
    {
    return m_geometryListBuilder.AddBody(solid, CreateDefaultDisplayParams(gf), gf.GetLocalToWorldTransform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessTextString(TextStringCR textString, SimplifyGraphic& gf) 
    {
    return m_geometryListBuilder.AddTextString(textString, CreateDisplayParams(gf, true /*ignore lighting*/), gf.GetLocalToWorldTransform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometryProcessor::AddElementGeometry(GeometryR geom)
    {
    // ###TODO: Only if geometry caching enabled...
    m_geometryListBuilder.AddGeometry(geom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometryProcessor::PushGeometry(GeometryR geom)
    {
    if (_AcceptGeometry(geom, m_geometryListBuilder.GetElementId()))
        m_geometries.push_back(&geom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_AcceptGeometry(GeometryCR geom, DgnElementId elemId)
    {
    if (BelowMinRange(geom.GetTileRange()))
        {
        // It's possible only some portions of an element's geometry are excluded by range.
        if (!m_anyCurvedGeometry)
            m_excludedElements.insert(elemId);

        return false;
        }

    if (!m_anyCurvedGeometry)
        m_anyCurvedGeometry = geom.IsCurved();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_AcceptElement(DRange3dCR range, DgnElementId elemId)
    {
    if (!BelowMinRange(range))
        return true;

    if (!m_anyCurvedGeometry)
        m_excludedElements.insert(elemId);

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometryProcessor::AddGeomPart (Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, GeometryParamsR geomParams, GraphicParamsR graphicParams, ViewContextR viewContext)
    {
    Transform partToWorld = Transform::FromProduct(graphic.GetLocalToWorldTransform(), subToGraphic);
    DisplayParamsCR displayParams = m_geometryListBuilder.GetDisplayParamsCache().Get(graphicParams, geomParams);
    DRange3d range;

    GeomPartPtr             tileGeomPart = m_root.GetGeomPart(partId);
    if (tileGeomPart.IsNull())
        {
        DgnGeometryPartCPtr geomPart = GetDgnDb().Elements().Get<DgnGeometryPart>(partId);

        if (!geomPart.IsValid())
            return;

        Transform                       inverseLocalToWorld;
        Transform                       builderTransform = m_geometryListBuilder.GetTransform();
        m_geometryListBuilder.SetTransform(Transform::FromIdentity());

        GeometryStreamIO::Collection    collection(geomPart->GetGeometryStream().GetData(), geomPart->GetGeometryStream().GetSize());
        
        inverseLocalToWorld.InverseOf (graphic.GetLocalToWorldTransform());

        auto                            partBuilder = graphic.CreateSubGraphic(inverseLocalToWorld);
        GeometryList                    saveCurrGeometries = m_geometryListBuilder.GetGeometries();;;
        
        m_geometryListBuilder.Clear();
        collection.Draw(*partBuilder, viewContext, geomParams, false, geomPart.get());

        if (viewContext._CheckStop())
            return;

        m_root.AddGeomPart (partId, *(tileGeomPart = GeomPart::Create(geomPart->GetBoundingBox(), m_geometryListBuilder.GetGeometries())));

        m_geometryListBuilder.SetGeometryList(saveCurrGeometries);
        m_geometryListBuilder.SetTransform(builderTransform);
        }

    Transform   tf = Transform::FromProduct(GetTransformFromDgn(), partToWorld);
    
    tf.Multiply(range, tileGeomPart->GetRange());
    AddElementGeometry(*Geometry::Create(*tileGeomPart, tf, range, GetCurrentElementId(), displayParams, GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryProcessor::Result TileGeometryProcessor::OutputGraphics(GeometryProcessorContext& context)
    {
    GeometryCollector collector(m_range, *this, context);
    auto status = collector.Collect();
    if (Result::Aborted == status)
        m_geometries.clear();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::ViewFlags GeometryProcessorContext::GetDefaultViewFlags()
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
void GeometryProcessorContext::_AddSubGraphic(Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, GeometryParamsR geomParams)
    {
    GraphicParams graphicParams;
    _CookGeometryParams(geomParams, graphicParams);
    m_processor->AddGeomPart(graphic, partId, subToGraphic, geomParams, graphicParams, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GeometryProcessorContext::_VisitElement(DgnElementId elementId, bool allowLoad)
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
    m_processor->GetRoot().GetDbMutex().Enter();
    StatusInt status = ERROR;
    auto& stmt = *m_statement;
    stmt.BindInt64(1, static_cast<int64_t>(elementId.GetValueUnchecked()));

    if (BeSQLite::BE_SQLITE_ROW == stmt.Step() && !IsValueNull(1))
        {
        auto geomSrcPtr = m_is3dView ? GeometrySelector3d::ExtractGeometrySource(stmt, GetDgnDb()) : GeometrySelector2d::ExtractGeometrySource(stmt, GetDgnDb());

        stmt.Reset();
        m_processor->GetRoot().GetDbMutex().Leave();

        if (nullptr != geomSrcPtr)
            status = VisitGeometry(*geomSrcPtr);
        }
    else
        {
        stmt.Reset();
        m_processor->GetRoot().GetDbMutex().Leave();
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr GeometryProcessorContext::_StrokeGeometry(GeometrySourceCR source, double pixelSize)
    {
    Render::GraphicPtr graphic = source.Draw(*this, pixelSize);
    return WasAborted() ? nullptr : graphic;
    }

#if defined (BENTLEYCONFIG_PARASOLID) 
// ###TODO: ugh.
static ThreadedLocalParasolidHandlerStorageMark s_tempParasolidThreadedHandlerStorageMark;
#endif


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

#define COMPARE_VALUES_TOLERANCE(val0, val1, tol)   if (val0 < val1 - tol) return true; if (val0 > val1 + tol) return false;
#define COMPARE_VALUES(val0, val1) if (val0 < val1) { return true; } if (val0 > val1) { return false; }

#if defined(NDEBUG)
#define ELEMENT_TILE_DEBUG_RANGE false
#else
#define ELEMENT_TILE_DEBUG_RANGE true
#endif

USING_NAMESPACE_ELEMENT_TILETREE

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
static FPoint3d toFPoint3d(DPoint3dCR dpoint)
    {
    FPoint3d fpoint;
    fpoint.x = dpoint.x;
    fpoint.y = dpoint.y;
    fpoint.z = dpoint.z;
    return fpoint;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
static FPoint2d toFPoint2d(DPoint2dCR dpoint)
    {
    FPoint2d fpoint;
    fpoint.x = dpoint.x;
    fpoint.y = dpoint.y;
    return fpoint;
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
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void collectCurveStrokes (bvector<bvector<DPoint3d>>& strokes, CurveVectorCR curve, IFacetOptionsR facetOptions, TransformCR transform)
    {                    
    bvector <bvector<bvector<DPoint3d>>> strokesArray;

    curve.CollectLinearGeometry (strokesArray, &facetOptions);

    for (auto& loop : strokesArray)
        {
        for (auto& loopStrokes : loop)
            {
            transform.Multiply(loopStrokes, loopStrokes);
            strokes.push_back (std::move(loopStrokes));
            }
        }
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   01/17
//=======================================================================================
struct TileInstances : IGraphicBuilder::Instances
{
    bvector<Transform>  m_meshTransforms;

    TileInstances() : IGraphicBuilder::Instances(0, nullptr) { }

    void Clear()
        {
        m_meshTransforms.clear();
        m_count = 0;
        m_transforms = nullptr;
        }

    void Init(MeshInstanceList const& meshInstances)
        {
        Clear();
        m_meshTransforms.reserve(meshInstances.size());
        for (auto const& meshInstance : meshInstances)
            m_meshTransforms.push_back(meshInstance.GetTransform());

        m_count = static_cast<uint32_t>(m_meshTransforms.size());
        m_transforms = &m_meshTransforms[0];
        }
};

//=======================================================================================
// ###TODO? We don't want to have to copy all the indices into a new buffer...store
// them that way in Mesh struct?
// OTOH we do want to support single- or double-sided triangles...not clear if QVis API currently
// supports that.
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct TileMeshArgs : IGraphicBuilder::TriMeshArgs
{
    bvector<int32_t>   m_indices;

    template<typename T, typename U> void Set(int32_t& count, T& ptr, U const& src)
        {
        count = static_cast<int32_t>(src.size());
        Set(ptr, src);
        }

    template<typename T, typename U> void Set(T& ptr, U const& src)
        {
        ptr = 0 != src.size() ? src.data() : nullptr;
        }

    void Clear()
        {
        m_indices.clear();
        m_numIndices = 0;
        m_vertIndex = nullptr;
        m_numPoints = 0;
        m_points = nullptr;
        m_normals = nullptr;
        m_textureUV = nullptr;
        m_texture = nullptr;
        m_flags = 0;
        }

    bool Init(ElementTileTree::MeshCR mesh, Render::System const& system, DgnDbP db)
        {
        Clear();

        if (mesh.Triangles().empty())
            return false;

        for (auto const& triangle : mesh.Triangles())
            {
            m_indices.push_back(static_cast<int32_t>(triangle.m_indices[0]));
            m_indices.push_back(static_cast<int32_t>(triangle.m_indices[1]));
            m_indices.push_back(static_cast<int32_t>(triangle.m_indices[2]));
            }

        Set(m_numIndices, m_vertIndex, m_indices);
        Set(m_numPoints, m_points, mesh.Points());
        Set(m_textureUV, mesh.Params());
        if (!mesh.GetDisplayParams().GetIgnoreLighting())    // ###TODO: Avoid generating normals in the first place if no lighting...
            Set(m_normals, mesh.Normals());

        auto const& displayParams = mesh.GetDisplayParams();
        displayParams.ResolveTextureImage(db);
        if (nullptr != displayParams.GetTextureImage())
            m_texture = system._CreateTexture(displayParams.GetTextureImage()->GetImageSource(), Render::Image::Format::Rgba, Render::Image::BottomUp::No);

        return true;
        }

    void Transform(TransformCR tf)
        {
        for (int32_t i = 0; i < m_numPoints; i++)
            {
            FPoint3d& fpt = const_cast<FPoint3d&>(m_points[i]);
            DPoint3d dpt = DPoint3d::FromXYZ(fpt.x, fpt.y, fpt.z);
            tf.Multiply(dpt);
            fpt.x = dpt.x;
            fpt.y = dpt.y;
            fpt.z = dpt.z;

            if (nullptr != m_normals)
                {
                FPoint3d& fnm = const_cast<FPoint3d&>(m_normals[i]);
                dpt = DPoint3d::FromXYZ(fnm.x, fnm.y, fnm.z);
                tf.MultiplyMatrixOnly(dpt);
                fnm.x = dpt.x;
                fnm.y = dpt.y;
                fnm.z = dpt.z;
                }
            }
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct IndexedPolyline : IGraphicBuilder::IndexedPolylineArgs::Polyline
{
    bool IsValid() const { return 0 < m_numIndices; }

    void Reset()
        {
        m_numIndices = 0;
        m_vertIndex = nullptr;
        }

    bool Init(ElementTileTree::PolylineCR line)
        {
        Reset();

        m_numIndices = static_cast<uint32_t>(line.GetIndices().size());
        m_vertIndex = &line.GetIndices()[0];

        return IsValid();
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct TilePolylineArgs : IGraphicBuilder::IndexedPolylineArgs
{
    bvector<IndexedPolyline>    m_polylines;

    bool IsValid() const { return !m_polylines.empty(); }

    void Reset()
        {
        m_numPoints = m_numLines = 0;
        m_points = nullptr;
        m_lines = nullptr;
        m_polylines.clear();
        }

    bool Init(ElementTileTree::MeshCR mesh)
        {
        Reset();

        m_numPoints = static_cast<uint32_t>(mesh.Points().size());
        m_points = &mesh.Points()[0];
        m_polylines.reserve(mesh.Polylines().size());

        for (auto const& polyline : mesh.Polylines())
            {
            IndexedPolyline indexedPolyline;
            if (indexedPolyline.Init(polyline))
                m_polylines.push_back(indexedPolyline);
            }

        if (IsValid())
            {
            m_numLines = static_cast<uint32_t>(m_polylines.size());
            m_lines = &m_polylines[0];
            }

        return IsValid();
        }

    void Apply(Render::GraphicBuilderR gf)
        {
        if (IsValid())
            gf.AddIndexedPolylines(*this);
        }

    bool InitAndApply(Render::GraphicBuilderR gf, ElementTileTree::MeshCR mesh)
        {
        if (Init(mesh))
            {
            Apply(gf);
            return true;
            }

        return false;
        }

    void Transform(TransformCR tf)
        {
        for (uint32_t i = 0; i < m_numPoints; i++)
            {
            FPoint3d fpt = m_points[i];
            DPoint3d dpt = DPoint3d::FromXYZ(fpt.x, fpt.y, fpt.z);
            tf.Multiply(dpt);
            fpt.x = dpt.x;
            fpt.y = dpt.y;
            fpt.z = dpt.z;
            }
        }
};

#if defined(ELEMENT_TILE_EXPAND_2D_RANGE)
constexpr double s_half2dDepthRange = 10.0;
#endif
constexpr double s_minRangeBoxSize    = 0.5;     // Threshold below which we consider geometry/element too small to contribute to tile mesh
constexpr size_t s_maxGeometryIdCount = 0xffff;  // Max batch table ID - 16-bit unsigned integers
constexpr double s_minToleranceRatio = 1000.0;
constexpr uint32_t s_minElementsPerTile = 50;
constexpr size_t s_maxPointsPerTile = 10000;
constexpr size_t s_minLeafTolerance = 0.001;
constexpr double s_solidPrimitivePartCompareTolerance = 1.0E-5;
constexpr double s_vertexToleranceRatio    = .1;
constexpr double s_facetAreaToleranceRatio = .1;

enum class InstancingOptions
{
    // Add each instance as a top-level graphic.
    // Downside: Scene creation is slower as we have to populate much larger lists of graphics
    AsGraphics,
    // Add each instance as a sub-graphic with a transform
    // Downside: qv_pushTransclip() is inordinately slow.
    // Upside: we're no longer using QVis, so downside is inapplicable
    AsSubGraphics,
    // Transform instanced geometry in place, then add as a subgraphic with identity transform
    // Downside: Separate QvElem for each subgraphic; slower tile generation due to applying the transform. Probably best bet for now
    AsPreTransformedSubGraphics,
    // Use the AddInstanced* functions
    AsInstances,
};

// avoid re-facetting repeated geometry - cache and reuse
// Improves tile generation time - but that was before we enabled concurrent parasolid facetting. Requires mutexes, additional state - may not be worth it.
static bool s_cacheInstances = false;
static InstancingOptions s_instancingOptions = InstancingOptions::AsInstances;
static Render::GraphicSet s_unusedDummyGraphicSet;

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
struct RangeAccumulator : RangeIndex::Traverser
{
    DRange3dR       m_range;
    bool            m_is2d;

    RangeAccumulator(DRange3dR range, bool is2d) : m_range(range), m_is2d(is2d) { m_range = DRange3d::NullRange(); }

    virtual bool _AbortOnWriteRequest() const override { return true; }
    virtual Accept _CheckRangeTreeNode(RangeIndex::FBoxCR, bool) const override { return Accept::Yes; }
    virtual Stop _VisitRangeTreeEntry(RangeIndex::EntryCR entry) override
        {
        m_range.Extend(entry.m_range.ToRange3d());
        return Stop::No;
        }

    bool Accumulate(RangeIndex::Tree& tree)
        {
        if (Stop::Yes == tree.Traverse(*this))
            return false;
        else if (m_range.IsNull())
            return false;

#if defined(ELEMENT_TILE_EXPAND_2D_RANGE)
        // ###TODO_ELEMENT_TILE: This was required for the tile publisher because Cesium does not support true 2d views.
        if (m_is2d)
            {
            BeAssert(m_range.low.z == m_range.high.z == 0.0);
            m_range.low.z = -s_half2dDepthRange*2;  // times 2 so we don't stick geometry right on the boundary...
            m_range.high.z = s_half2dDepthRange*2;
            }
#endif

        return true;
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct PrimitiveGeometry : Geometry
{
private:
    IGeometryPtr        m_geometry;

    PrimitiveGeometry(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsR params, bool isCurved, DgnDbP db)
        : Geometry(tf, range, elemId, params, isCurved, db), m_geometry(&geometry) { }

    virtual PolyfaceList _GetPolyfaces(IFacetOptionsR facetOptions) override;
    virtual StrokesList _GetStrokes (IFacetOptionsR facetOptions) override;
    virtual bool _DoDecimate () const override { return m_geometry->GetAsPolyfaceHeader().IsValid(); }
    virtual size_t _GetFacetCount(FacetCounter& counter) const override { return counter.GetFacetCount(*m_geometry); }
public:
    static GeometryPtr Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsR params, bool isCurved, DgnDbP db)
        {
        return new PrimitiveGeometry(geometry, tf, range, elemId, params, isCurved, db);
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct SolidKernelGeometry : Geometry
{
private:
    IBRepEntityPtr      m_entity;
    BeMutex             m_mutex;

    SolidKernelGeometry(IBRepEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsR params, DgnDbP db)
        : Geometry(tf, range, elemId, params, BRepUtil::HasCurvedFaceOrEdge(solid), db), m_entity(&solid) { }

    virtual PolyfaceList _GetPolyfaces(IFacetOptionsR facetOptions) override;
    virtual size_t _GetFacetCount(FacetCounter& counter) const override { return counter.GetFacetCount(*m_entity); }
public:
    static GeometryPtr Create(IBRepEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsR params, DgnDbP db)
        {
        return new SolidKernelGeometry(solid, tf, range, elemId, params, db);
        }
};

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     11/2016
//=======================================================================================
struct TextStringGeometry : Geometry
{
private:
    TextStringPtr                   m_text;
    mutable bvector<CurveVectorPtr> m_glyphCurves;

    TextStringGeometry(TextStringR text, TransformCR transform, DRange3dCR range, DgnElementId elemId, DisplayParamsR params, DgnDbP db)
        : Geometry(transform, range, elemId, params, true, db), m_text(&text) 
        { 
        InitGlyphCurves();     // Should be able to defer this when font threaded ness is resolved.
        }

    virtual bool _DoVertexCluster() const override { return false; }

public:
    static GeometryPtr Create(TextStringR textString, TransformCR transform, DRange3dCR range, DgnElementId elemId, DisplayParamsR params, DgnDbP db)
        {
        return new TextStringGeometry(textString, transform, range, elemId, params, db);
        }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool     DoGlyphBoxes (IFacetOptionsR facetOptions)
    {
    DRange2d            textRange = m_text->GetRange();
    double              minDimension = std::min (textRange.high.x - textRange.low.x, textRange.high.y - textRange.low.y) * GetTransform().ColumnXMagnitude();
    static const double s_minGlyphRatio = 1.0; 
    
    return minDimension < s_minGlyphRatio * facetOptions.GetChordTolerance();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
virtual PolyfaceList _GetPolyfaces(IFacetOptionsR facetOptions) override
    {

    PolyfaceList                polyfaces;
    IPolyfaceConstructionPtr    polyfaceBuilder = IPolyfaceConstruction::Create(facetOptions);

    if (DoGlyphBoxes(facetOptions))
        {
        DVec3d              xAxis, yAxis;
        DgnGlyphCP const*   glyphs = m_text->GetGlyphs();
        DPoint3dCP          glyphOrigins = m_text->GetGlyphOrigins();

        m_text->ComputeGlyphAxes(xAxis, yAxis);
        Transform       rotationTransform = Transform::From (RotMatrix::From2Vectors(xAxis, yAxis));

        for (size_t iGlyph = 0; iGlyph <  m_text->GetNumGlyphs(); ++iGlyph)
            {
            if (nullptr != glyphs[iGlyph])
                {
                DRange2d                range = glyphs[iGlyph]->GetExactRange();
                bvector<DPoint3d>       box(5);

                box[0].x = box[3].x = box[4].x = range.low.x;
                box[1].x = box[2].x = range.high.x;

                box[0].y = box[1].y = box[4].y = range.low.y;
                box[2].y = box[3].y = range.high.y;

                Transform::FromProduct (Transform::From(glyphOrigins[iGlyph]), rotationTransform).Multiply (box, box);

                polyfaceBuilder->AddTriangulation (box);
                }
            }
        }
    else
        {
        for (auto& glyphCurve : m_glyphCurves)
            polyfaceBuilder->AddRegion(*glyphCurve);
        }

    PolyfaceHeaderPtr   polyface = polyfaceBuilder->GetClientMeshPtr();

    if (polyface.IsValid() && polyface->HasFacets())
        {
        polyface->Transform(Transform::FromProduct (GetTransform(), m_text->ComputeTransform()));
        polyfaces.push_back (Polyface(*GetDisplayParamsPtr(), *polyface));
        }

    return polyfaces;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StrokesList _GetStrokes (IFacetOptionsR facetOptions) override
    {
    StrokesList strokes;

    if (DoGlyphBoxes(facetOptions))
        return strokes;

    InitGlyphCurves();

    bvector<bvector<DPoint3d>>  strokePoints;
    Transform                   transform = Transform::FromProduct (GetTransform(), m_text->ComputeTransform());

    for (auto& glyphCurve : m_glyphCurves)
        if (!glyphCurve->IsAnyRegionType())
            collectCurveStrokes(strokePoints, *glyphCurve, facetOptions, transform);

    if (!strokePoints.empty())
        strokes.push_back(Strokes(*GetDisplayParamsPtr(), std::move(strokePoints)));

    return strokes;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
virtual size_t _GetFacetCount(FacetCounter& counter) const override 
    { 
    InitGlyphCurves();
    size_t              count = 0;

    for (auto& glyphCurve : m_glyphCurves)
        count += counter.GetFacetCount(*glyphCurve);

    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/    
void  InitGlyphCurves() const
    {
    if (!m_glyphCurves.empty())
        return;

    DVec3d              xAxis, yAxis;
    DgnGlyphCP const*   glyphs = m_text->GetGlyphs();
    DPoint3dCP          glyphOrigins = m_text->GetGlyphOrigins();

    m_text->ComputeGlyphAxes(xAxis, yAxis);
    Transform       rotationTransform = Transform::From (RotMatrix::From2Vectors(xAxis, yAxis));

    for (size_t iGlyph = 0; iGlyph <  m_text->GetNumGlyphs(); ++iGlyph)
        {
        if (nullptr != glyphs[iGlyph])
            {
            bool            isFilled = false;
            CurveVectorPtr  glyphCurveVector = glyphs[iGlyph]->GetCurveVector(isFilled);

            if (glyphCurveVector.IsValid())
                {
                glyphCurveVector->TransformInPlace (Transform::FromProduct (Transform::From(glyphOrigins[iGlyph]), rotationTransform));
                m_glyphCurves.push_back(glyphCurveVector);
                }
            }
        }                                                                                                           
    }
};  // TextStringGeometry

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     11/2016
//=======================================================================================
struct GeomPartInstanceGeometry : Geometry
{
private:
    GeomPartPtr     m_part;

    GeomPartInstanceGeometry(GeomPartR  part, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsR params, DgnDbP db)
        : Geometry(tf, range, elemId, params, part.IsCurved(), db), m_part(&part) { }
public:
    static GeometryPtr Create(GeomPartR  part, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsR params, DgnDbP db)  { return new GeomPartInstanceGeometry(part, tf, range, elemId, params, db); }

    virtual PolyfaceList _GetPolyfaces(IFacetOptionsR facetOptions) override { return m_part->GetPolyfaces(facetOptions, *this); }
    virtual StrokesList _GetStrokes (IFacetOptionsR facetOptions) override { return m_part->GetStrokes(facetOptions, *this); }
    virtual size_t _GetFacetCount(FacetCounter& counter) const override { return m_part->GetFacetCount (counter, *this); }
    virtual GeomPartCPtr _GetPart() const override { return m_part; }

};  // GeomPartInstanceTileGeometry 

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

    virtual DgnDbR _GetSourceDgnDb() const override { return m_db; }
    virtual DgnElementCP _ToElement() const override { return nullptr; }
    virtual GeometrySource3dCP _GetAsGeometrySource3d() const override { return this; }
    virtual DgnCategoryId _GetCategoryId() const override { return m_categoryId; }
    virtual GeometryStreamCR _GetGeometryStream() const override { return m_geom; }
    virtual Placement3dCR _GetPlacement() const override { return m_placement; }

    virtual Render::GraphicSet& _Graphics() const override { BeAssert(false && "No reason to access this"); return s_unusedDummyGraphicSet; }
    virtual DgnDbStatus _SetCategoryId(DgnCategoryId categoryId) override { BeAssert(false && "No reason to access this"); return DgnDbStatus::BadRequest; }
    virtual DgnDbStatus _SetPlacement(Placement3dCR) override { BeAssert(false && "No reason to access this"); return DgnDbStatus::BadRequest; }
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

    virtual DgnDbR _GetSourceDgnDb() const override { return m_db; }
    virtual DgnElementCP _ToElement() const override { return nullptr; }
    virtual GeometrySource2dCP _GetAsGeometrySource2d() const override { return this; }
    virtual DgnCategoryId _GetCategoryId() const override { return m_categoryId; }
    virtual GeometryStreamCR _GetGeometryStream() const override { return m_geom; }
    virtual Placement2dCR _GetPlacement() const override { return m_placement; }

    virtual Render::GraphicSet& _Graphics() const override { BeAssert(false && "No reason to access this"); return s_unusedDummyGraphicSet; }
    virtual DgnDbStatus _SetCategoryId(DgnCategoryId categoryId) override { BeAssert(false && "No reason to access this"); return DgnDbStatus::BadRequest; }
    virtual DgnDbStatus _SetPlacement(Placement2dCR) override { BeAssert(false && "No reason to access this"); return DgnDbStatus::BadRequest; }
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
    IFacetOptionsPtr            m_lsStrokerOptions;
    RootR                       m_root;
    GeometryList&               m_geometries;
    DRange3d                    m_range;
    DRange3d                    m_tileRange;
    GeometryListBuilder         m_geometryListBuilder;
#if defined(ELEMENT_TILE_REGENERATION)
    TileModelDeltaP             m_modelDelta;
#endif
    double                      m_minRangeDiagonal;
    double                      m_minTextBoxSize;
    double                      m_tolerance;
    bool*                       m_leafThresholdExceeded;
    size_t                      m_leafCountThreshold;
    size_t                      m_leafCount;
    bool                        m_is2d;

    void PushGeometry(GeometryR geom);
    void AddElementGeometry(GeometryR geom);

    virtual IFacetOptionsP _GetFacetOptionsP() override { return &m_facetOptions; }

    virtual bool _ProcessCurveVector(CurveVectorCR curves, bool filled, SimplifyGraphic& gf) override;
    virtual bool _ProcessSolidPrimitive(ISolidPrimitiveCR prim, SimplifyGraphic& gf) override;
    virtual bool _ProcessSurface(MSBsplineSurfaceCR surface, SimplifyGraphic& gf) override;
    virtual bool _ProcessPolyface(PolyfaceQueryCR polyface, bool filled, SimplifyGraphic& gf) override;
    virtual bool _ProcessBody(IBRepEntityCR solid, SimplifyGraphic& gf) override;
    virtual bool _ProcessTextString(TextStringCR, SimplifyGraphic&) override;

    virtual double _AdjustZDepth(double zDepthIn) override
        {
#if defined(ELEMENT_TILE_EXPAND_2D_RANGE)
        // zDepth is obtained from GeometryParams::GetNetDisplayPriority(), which returns an int32_t.
        // Coming from mstn, priorities tend to be in [-500..500]
        // Let's assume that mstn's range is the full range and clamp anything outside that.
        // Map them to [-s_half2dDepthRange, s_half2dDepthRange]
        constexpr double priorityRange = 500;
        constexpr double ratio = s_half2dDepthRange / priorityRange;

        auto zDepth = std::min(zDepthIn, priorityRange);
        zDepth = std::max(zDepth, -priorityRange);

        return zDepth * ratio;
#else
        return Target::DepthFromDisplayPriority(static_cast<int32_t>(zDepthIn));
#endif
        }

    virtual UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR, SimplifyGraphic&) const override { return UnhandledPreference::Facet; }
    virtual UnhandledPreference _GetUnhandledPreference(CurveVectorCR, SimplifyGraphic&)     const override { return UnhandledPreference::Facet; }
    virtual UnhandledPreference _GetUnhandledPreference(IBRepEntityCR, SimplifyGraphic&)     const override { return UnhandledPreference::Facet; }

public:
#if defined(ELEMENT_TILE_REGENERATION)
    TileGeometryProcessor(GeometryList& geometries, RootR root, DRange3dCR range, IFacetOptionsR facetOptions, TransformCR transformFromDgn, TileModelDeltaP modelDelta, bool* leafThresholdExceeded, double tolerance, bool surfacesOnly, size_t leafCountThreshold, bool is2d) 
#else
    TileGeometryProcessor(GeometryList& geometries, RootR root, DRange3dCR range, IFacetOptionsR facetOptions, TransformCR transformFromDgn, bool* leafThresholdExceeded, double tolerance, bool surfacesOnly, size_t leafCountThreshold, bool is2d) 
#endif
      : m_geometries (geometries), m_facetOptions(facetOptions), m_targetFacetOptions(facetOptions.Clone()), m_root(root), m_range(range), m_geometryListBuilder(&root.GetDgnDb(), transformFromDgn, surfacesOnly), 
#if defined(ELEMENT_TILE_REGENERATION)
        m_modelDelta(modelDelta),
#endif
        m_tolerance(tolerance), m_leafThresholdExceeded(leafThresholdExceeded), m_leafCountThreshold(leafCountThreshold), m_leafCount(0), m_is2d(is2d)
        {
        static const double s_minTextBoxSize = 1.0;     // Below this ratio to tolerance  text is rendered as box.

        m_targetFacetOptions->SetChordTolerance(facetOptions.GetChordTolerance() * transformFromDgn.ColumnXMagnitude());
        m_minRangeDiagonal = s_minRangeBoxSize * tolerance;
        m_minTextBoxSize  = s_minTextBoxSize * tolerance;
        GetTransformFromDgn().Multiply (m_tileRange, m_range);
        }

    void ProcessElement(ViewContextR context, DgnElementId elementId, DRange3dCR range);
    Result OutputGraphics(GeometryProcessorContext& context);
    void AddGeomPart (Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, GeometryParamsR geomParams, GraphicParamsR graphicParams, ViewContextR viewContext);


    DgnDbR GetDgnDb() const { return m_root.GetDgnDb(); }
    RootR GetRoot() const { return m_root; }

    bool BelowMinRange(DRange3dCR range) const
        {
        // Avoid processing any elements with range smaller than roughly half a pixel...
        return range.DiagonalDistance() < m_minRangeDiagonal;
        }

    IFacetOptionsPtr GetLineStyleStrokerOptions(LineStyleSymbCR lsSymb)
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

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Ray.Bentley     11/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual UnhandledPreference _GetUnhandledPreference(TextStringCR textString, SimplifyGraphic& simplifyGraphic) const override 
        {
        DRange2d        range = textString.GetRange();
        Transform       transformToTile = Transform::FromProduct(GetTransformFromDgn(), simplifyGraphic.GetLocalToWorldTransform(), textString.ComputeTransform());
        double          minTileDimension = transformToTile.ColumnXMagnitude() * std::min(range.XLength(), range.YLength());

        return minTileDimension < m_minTextBoxSize ? UnhandledPreference::Box : UnhandledPreference::Curve;
        }

    bool WantSurfacesOnly() const { return m_geometryListBuilder.WantSurfacesOnly(); }
    DgnElementId GetCurrentElementId() const { return m_geometryListBuilder.GetElementId(); }
    TransformCR GetTransformFromDgn() const { return m_geometryListBuilder.GetTransform(); }
    DisplayParamsPtr CreateDisplayParams(SimplifyGraphic& gf, bool ignoreLighting) const { return DisplayParams::Create(gf.GetCurrentGraphicParams(), gf.GetCurrentGeometryParams(), ignoreLighting); }
    DisplayParamsPtr CreateDefaultDisplayParams(SimplifyGraphic& gf) const { return CreateDisplayParams(gf, m_is2d); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
struct GeometryProcessorContext : NullContext
{
DEFINE_T_SUPER(NullContext);

private:
    TileGeometryProcessor&          m_processor;
    BeSQLite::CachedStatementPtr    m_statement;
    LoadContextCR                   m_loadContext;

    bool IsValueNull(int index) { return m_statement->IsColumnNull(index); }

    virtual Render::GraphicBuilderPtr _CreateGraphic(Render::Graphic::CreateParams const& params) override
        {
        return new SimplifyGraphic(params, m_processor, *this);
        }

    virtual StatusInt _VisitElement(DgnElementId elementId, bool allowLoad) override;
    virtual Render::GraphicPtr _StrokeGeometry(GeometrySourceCR, double) override;
    virtual Render::GraphicPtr _AddSubGraphic(Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, GeometryParamsR geomParams) override;
    virtual bool _CheckStop() override { return WasAborted() || AddAbortTest(m_loadContext.WasAborted()); }

    bool _UseLineStyleStroker(Render::GraphicBuilderR builder, LineStyleSymbCR lsSymb, IFacetOptionsPtr& facetOptions) const override
        {
        facetOptions = m_processor.GetLineStyleStrokerOptions(lsSymb);
        return facetOptions.IsValid();
        }

    Render::MaterialPtr _GetMaterial(DgnMaterialId id) const override
        {
        Render::SystemP system = m_processor.GetRoot().GetRenderSystem();
        return nullptr != system ? system->_GetMaterial(id, m_processor.GetDgnDb()) : nullptr;
        }
public:
    GeometryProcessorContext(TileGeometryProcessor& processor, RootR root, LoadContextCR loadContext)
        : m_processor(processor), m_statement(root.GetDgnDb().GetCachedStatement(root.Is3d() ? GeometrySelector3d::GetSql() : GeometrySelector2d::GetSql())), m_loadContext(loadContext)
        {
        SetDgnDb(root.GetDgnDb());
        m_is3dView = root.Is3d(); // force Brien to call _AddArc2d() if we're in a 2d model...
        }

    bool AcceptElement(DgnElementId elem, DgnCategoryId cat) const { return m_loadContext.AcceptElement(elem, cat); }
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

    virtual Accept _CheckRangeTreeNode(RangeIndex::FBoxCR box, bool is3d) const override
        {
        return !m_context.CheckStop() && box.IntersectsWith(m_range) ? Accept::Yes : Accept::No;
        }

    virtual Stop _VisitRangeTreeEntry(RangeIndex::EntryCR entry) override
        {
        if (m_context.CheckStop())
            return Stop::Yes;

        if (entry.m_range.IntersectsWith(m_range) && m_context.AcceptElement(entry.m_id, entry.m_category))
            {
            auto entryRange = entry.m_range.ToRange3d();
            if (!m_processor.BelowMinRange(entryRange))
                m_processor.ProcessElement(m_context, entry.m_id, entryRange);
            }

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

#define ELEMENT_TILE_TRUNCATE_PLANAR
#if defined(ELEMENT_TILE_CHECK_FACET_COUNTS) || defined(ELEMENT_TILE_TRUNCATE_PLANAR)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void adjustGeometryTolerance(GeometryList& geometries, double tolerance)
    {
    // Remove any geometry too small for inclusion in at this tolerance
    double minRangeDiagonal = s_minRangeBoxSize * tolerance;
    auto eraseAt = std::remove_if(geometries.begin(), geometries.end(), [=](GeometryPtr const& geom) { return geom->GetTileRange().DiagonalDistance() < minRangeDiagonal; });
    if (eraseAt != geometries.end())
        geometries.erase(eraseAt, geometries.end());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static IFacetOptionsPtr createTileFacetOptions(double chordTolerance)
    {
    static double       s_defaultAngleTolerance = msGeomConst_piOver2;
    IFacetOptionsPtr    opts = IFacetOptions::Create();

    opts->SetChordTolerance(chordTolerance);
    opts->SetAngleTolerance(s_defaultAngleTolerance);
    opts->SetMaxPerFace(3);
    opts->SetCurvedSurfaceMaxPerFace(3);
    opts->SetParamsRequired(true);
    opts->SetNormalsRequired(true);

    return opts;
    }

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureCPtr DisplayParams::QueryTexture(DgnDbP db) const
    {
    if (nullptr == db)
        return nullptr;

    JsonRenderMaterial mat;
    if (!GetMaterialId().IsValid() || SUCCESS != mat.Load(GetMaterialId(), *db))
        return nullptr;

    auto texMap = mat.GetPatternMap();
    DgnTextureId texId;
    if (!texMap.IsValid() || !(texId = texMap.GetTextureId()).IsValid())
        return nullptr;

    return DgnTexture::Get(*db, texId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayParams::operator<(DisplayParamsCR rhs) const
    {
    COMPARE_VALUES (GetFillColor(), rhs.GetFillColor());
    COMPARE_VALUES (GetRasterWidth(), rhs.GetRasterWidth());                                                           
    COMPARE_VALUES (GetMaterialId().GetValueUnchecked(), rhs.GetMaterialId().GetValueUnchecked());

    // Note - do not compare category and subcategory - These are used only for 
    // extracting BRep face attachments.  Comparing them would create seperate
    // meshes for geometry with same symbology but different category.
    // This was determined (empirically) to degrade performance. 

    // No need to compare textures -- if materials match then textures must too.
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
ImageSource TextureImage::Load(DisplayParamsCR params, DgnDbR db)
    {
    DgnTextureCPtr tex = params.QueryTexture(&db);
    return tex.IsValid() ? tex->GetImageSource() : ImageSource();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayParams::ResolveTextureImage(DgnDbP db) const
    {
    if (m_textureImage.IsValid() || nullptr == db)
        return;

    ImageSource renderImage  = TextureImage::Load(*this, *db);

    if (renderImage.IsValid())
        m_textureImage = TextureImage::Create(std::move(renderImage));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d Mesh::GetDPoint3d(bvector<FPoint3d> const& from, uint32_t index) const
    {
    auto fpoint = from.at(index);
    return DPoint3d::FromXYZ(fpoint.x, fpoint.y, fpoint.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d Mesh::GetTriangleRange(TriangleCR triangle) const
    {
    return DRange3d::From(GetDPoint3d(m_points, triangle.m_indices[0]),
                          GetDPoint3d(m_points, triangle.m_indices[1]),
                          GetDPoint3d(m_points, triangle.m_indices[2]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3d Mesh::GetTriangleNormal(TriangleCR triangle) const
    {
    return DVec3d::FromNormalizedCrossProductToPoints(GetDPoint3d(m_points, triangle.m_indices[0]),
                                                      GetDPoint3d(m_points, triangle.m_indices[1]),
                                                      GetDPoint3d(m_points, triangle.m_indices[2]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool Mesh::HasNonPlanarNormals() const
    {
    if (m_normals.empty())
        return false;

    for (auto& triangle : m_triangles)
        if(!GetDPoint3d(m_normals, triangle.m_indices[0]).IsEqual(GetDPoint3d(m_normals, triangle.m_indices[1])) ||
            !GetDPoint3d(m_normals, triangle.m_indices[0]).IsEqual(GetDPoint3d(m_normals, triangle.m_indices[2])) ||
            !GetDPoint3d(m_normals, triangle.m_indices[1]).IsEqual(GetDPoint3d(m_normals, triangle.m_indices[2])))
            return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void    Mesh::AddMesh (MeshCR mesh)
    {
    if (mesh.m_points.empty() ||
        m_normals.empty() != mesh.m_normals.empty() ||
        m_uvParams.empty() != mesh.m_uvParams.empty() ||
        m_entityIds.empty() != mesh.m_entityIds.empty())
        {
        BeAssert (false && "add mesh empty or not compatible");
        }

    size_t      baseIndex = m_points.size();

    m_points.insert (m_points.end(), mesh.m_points.begin(), mesh.m_points.end());
    if (!mesh.m_normals.empty())
        m_normals.insert (m_normals.end(), mesh.m_normals.begin(), mesh.m_normals.end());

    if (!mesh.m_uvParams.empty())
        m_uvParams.insert (m_uvParams.end(), mesh.m_uvParams.begin(), mesh.m_uvParams.end());

    if (!mesh.m_entityIds.empty())
        m_entityIds.insert (m_entityIds.end(), mesh.m_entityIds.begin(), mesh.m_entityIds.end());

    for (auto& triangle : mesh.m_triangles)
        AddTriangle (Triangle (triangle.m_indices[0] + baseIndex, triangle.m_indices[1] + baseIndex, triangle.m_indices[2] + baseIndex, triangle.m_singleSided));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool    Mesh::RemoveEntityGeometry (bset<DgnElementId> const& deleteIds)
    {
    bool                        deleteGeometryFound = false;
    bmap<uint32_t, uint32_t>    indexRemap;
    bvector<FPoint3d>           savePoints = m_points;
    bvector<FPoint3d>           saveNormals = m_normals;
    bvector<FPoint2d>           saveParams = m_uvParams;
    bvector<DgnElementId>       saveEntityIds = m_entityIds;

    m_points.clear();
    m_normals.clear();
    m_uvParams.clear();
    m_entityIds.clear();

    for (size_t index = 0; index<saveEntityIds.size(); index++)
        {
        auto&   entityId = saveEntityIds[index];

        if (deleteIds.find(entityId) == deleteIds.end() &&
            indexRemap.find(index) == indexRemap.end())
            {
            indexRemap.Insert(index, (uint32_t) m_points.size());
            m_points.push_back(savePoints[index]);
            m_entityIds.push_back(entityId);
            if (!saveNormals.empty())
                m_normals.push_back(saveNormals[index]);
            if (!saveParams.empty())
                m_uvParams.push_back(saveParams[index]);
            }
        else
            {
            deleteGeometryFound = true;
            }
        }
    if (!deleteGeometryFound)
        return false;
        

    for (bvector<Triangle>::iterator  triangle = m_triangles.begin(); triangle != m_triangles.end(); )
        {
        if (indexRemap.find(triangle->m_indices[0]) == indexRemap.end())
            {
            m_triangles.erase (triangle);
            }
        else
            {
            for (size_t i=0; i<3; i++)
                triangle->m_indices[i] = indexRemap[triangle->m_indices[i]];

            triangle++;
            }
        }
    for (bvector<Polyline>::iterator  polyline = m_polylines.begin(); polyline != m_polylines.end(); )
        {
        auto& indices = polyline->GetIndices();
        if (indexRemap.find(indices[0]) == indexRemap.end())
            {
            m_polylines.erase (polyline);
            }
        else
            {
            for (size_t i=0; i<indices.size(); i++)
                indices[i] = indexRemap[indices[i]];

            polyline++;
            }                                                                                                         
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t Mesh::AddVertex(DPoint3dCR point, DVec3dCP normal, DPoint2dCP param, DgnElementId entityId)
    {
    auto index = static_cast<uint32_t>(m_points.size());

    m_points.push_back(toFPoint3d(point));
    m_entityIds.push_back(entityId);

    if (nullptr != normal)
        m_normals.push_back(toFPoint3d(*normal));
                                                                                                                 
    if (nullptr != param)
        m_uvParams.push_back(toFPoint2d(*param));

    m_validIdsPresent |= entityId.IsValid();
    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d Mesh::GetRange() const
    {
    DRange3d range = DRange3d::NullRange();
    for (auto const& fpoint : m_points)
        range.Extend(DPoint3d::FromXYZ(fpoint.x, fpoint.y, fpoint.z));

    return range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d Mesh::GetUVRange() const
    {
    DRange3d range = DRange3d::NullRange();
    for (auto const& fpoint : m_uvParams)
        range.Extend(DPoint3d::FromXYZ(fpoint.x, fpoint.y, 0.0));

    return range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TriangleKey::TriangleKey(TriangleCR triangle)
    {
    // Could just use std::sort - but this should be faster?
    if (triangle.m_indices[0] < triangle.m_indices[1])
        {
        if (triangle.m_indices[0] < triangle.m_indices[2])
            {
            m_sortedIndices[0] = triangle.m_indices[0];
            if (triangle.m_indices[1] < triangle.m_indices[2])
                {
                m_sortedIndices[1] = triangle.m_indices[1];
                m_sortedIndices[2] = triangle.m_indices[2];
                }
            else
                {
                m_sortedIndices[1] = triangle.m_indices[2];
                m_sortedIndices[2] = triangle.m_indices[1];
                }
            }
        else
            {
            m_sortedIndices[0] = triangle.m_indices[2];
            m_sortedIndices[1] = triangle.m_indices[0];
            m_sortedIndices[2] = triangle.m_indices[1];
            }
        }
    else
        {
        if (triangle.m_indices[1] < triangle.m_indices[2])
            {
            m_sortedIndices[0] = triangle.m_indices[1];
            if (triangle.m_indices[0] < triangle.m_indices[2])
                {
                m_sortedIndices[1] = triangle.m_indices[0];
                m_sortedIndices[2] = triangle.m_indices[2];
                }
            else
                {
                m_sortedIndices[1] = triangle.m_indices[2];
                m_sortedIndices[2] = triangle.m_indices[0];
                }
            }
        else
            {
            m_sortedIndices[0] = triangle.m_indices[2];
            m_sortedIndices[1] = triangle.m_indices[1];
            m_sortedIndices[2] = triangle.m_indices[0];
            }
        }

    BeAssert (m_sortedIndices[0] < m_sortedIndices[1]);
    BeAssert (m_sortedIndices[1] < m_sortedIndices[2]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TriangleKey::operator<(TriangleKeyCR rhs) const
    {
    COMPARE_VALUES (m_sortedIndices[0], rhs.m_sortedIndices[0]);
    COMPARE_VALUES (m_sortedIndices[1], rhs.m_sortedIndices[1]);
    COMPARE_VALUES (m_sortedIndices[2], rhs.m_sortedIndices[2]);

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool VertexKey::Comparator::operator()(VertexKeyCR lhs, VertexKeyCR rhs) const
    {
    static const double s_normalTolerance = .1;     
    static const double s_paramTolerance  = .1;

    COMPARE_VALUES (lhs.m_entityId, rhs.m_entityId);

    COMPARE_VALUES_TOLERANCE (lhs.m_point.x, rhs.m_point.x, m_tolerance);
    COMPARE_VALUES_TOLERANCE (lhs.m_point.y, rhs.m_point.y, m_tolerance);
    COMPARE_VALUES_TOLERANCE (lhs.m_point.z, rhs.m_point.z, m_tolerance);

    if (lhs.m_normalValid != rhs.m_normalValid)
        return rhs.m_normalValid;

    if (lhs.m_normalValid)
        {
        COMPARE_VALUES_TOLERANCE (lhs.m_normal.x, rhs.m_normal.x, s_normalTolerance);
        COMPARE_VALUES_TOLERANCE (lhs.m_normal.y, rhs.m_normal.y, s_normalTolerance);
        COMPARE_VALUES_TOLERANCE (lhs.m_normal.z, rhs.m_normal.z, s_normalTolerance);
        }

    if (lhs.m_paramValid != rhs.m_paramValid)
        return rhs.m_paramValid;

    if (lhs.m_paramValid)
        {
        COMPARE_VALUES_TOLERANCE (lhs.m_param.x, rhs.m_param.x, s_paramTolerance);
        COMPARE_VALUES_TOLERANCE (lhs.m_param.y, rhs.m_param.y, s_paramTolerance);
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilder::AddTriangle(TriangleCR triangle)
    {
    if (triangle.IsDegenerate())
        return;

    TriangleKey key(triangle);

    if (m_triangleSet.insert(key).second)
        m_mesh->AddTriangle(triangle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilder::AddTriangle(PolyfaceVisitorR visitor, DgnMaterialId materialId, DgnDbP dgnDb, DgnElementId entityId, bool doVertexCluster, bool duplicateTwoSidedTriangles, bool includeParams)
    {
    auto const&       points = visitor.Point();
    BeAssert(3 == points.size());

    if (doVertexCluster)
        {
        DVec3d      cross;

        cross.CrossProductToPoints (points.at(0), points.at(1), points.at(2));
        if (cross.MagnitudeSquared() < m_areaTolerance)
            return;
        }

    Triangle            newTriangle(!visitor.GetTwoSided());
    bvector<DPoint2d>   params = visitor.Param();

    if (includeParams &&
        !params.empty() &&
        nullptr != dgnDb &&
        (m_material.IsValid() || (materialId.IsValid() && SUCCESS == m_material.Load (materialId, *dgnDb))))
        {
        auto const&         patternMap = m_material.GetPatternMap();
        bvector<DPoint2d>   computedParams;

        if (patternMap.IsValid())
            {
            BeAssert (m_mesh->Points().empty() || !m_mesh->Params().empty());
            if (SUCCESS == patternMap.ComputeUVParams (computedParams, visitor))
                params = computedParams;
            }
        }
            
    bool haveNormals = !visitor.Normal().empty();
    for (size_t i = 0; i < 3; i++)
        {
        VertexKey vertex(points.at(i), haveNormals ? &visitor.Normal().at(i) : nullptr, !includeParams || params.empty() ? nullptr : &params.at(i), entityId);
        newTriangle.m_indices[i] = doVertexCluster ? AddClusteredVertex(vertex) : AddVertex(vertex);
        }

    BeAssert(m_mesh->Params().empty() || m_mesh->Params().size() == m_mesh->Points().size());
    BeAssert(m_mesh->Normals().empty() || m_mesh->Normals().size() == m_mesh->Points().size());

    AddTriangle(newTriangle);

    if (visitor.GetTwoSided() && duplicateTwoSidedTriangles)
        {
        Triangle dupTriangle(false);

        for (size_t i = 0; i < 3; i++)
            {
            size_t reverseIndex = 2 - i;
            DVec3d reverseNormal;
            if (haveNormals)
                reverseNormal.Negate(visitor.Normal().at(reverseIndex));

            VertexKey vertex(points.at(reverseIndex), haveNormals ? &reverseNormal : nullptr, includeParams || params.empty() ? nullptr : &params.at(reverseIndex), entityId);
            dupTriangle.m_indices[i] = doVertexCluster ? AddClusteredVertex(vertex) : AddVertex(vertex);
            }

        AddTriangle(dupTriangle);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilder::AddPolyline (bvector<DPoint3d>const& points, DgnElementId entityId, bool doVertexCluster)
    {
    Polyline    newPolyline;

    for (auto& point : points)
        {
        VertexKey vertex(point, nullptr, nullptr, entityId);

        newPolyline.GetIndices().push_back (doVertexCluster ? AddClusteredVertex(vertex) : AddVertex(vertex));
        }
    m_mesh->AddPolyline (newPolyline);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilder::AddPolyface (PolyfaceQueryCR polyface, DgnMaterialId materialId, DgnDbP dgnDb, DgnElementId entityId, bool twoSidedTriangles, bool includeParams)
    {
    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(polyface); visitor->AdvanceToNextFace(); )
        AddTriangle(*visitor, materialId, dgnDb, entityId, false, twoSidedTriangles, includeParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t MeshBuilder::AddVertex(VertexKey const& vertex)
    {
    auto found = m_unclusteredVertexMap.find(vertex);
    if (m_unclusteredVertexMap.end() != found)
        return found->second;

    auto index = m_mesh->AddVertex(vertex.m_point, vertex.GetNormal(), vertex.GetParam(), vertex.m_entityId);
    m_unclusteredVertexMap[vertex] = index;
    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t MeshBuilder::AddClusteredVertex(VertexKey const& vertex)
    {
    auto found = m_clusteredVertexMap.find(vertex);
    if (m_clusteredVertexMap.end() != found)
        return found->second;

    auto index = m_mesh->AddVertex(vertex.m_point, vertex.GetNormal(), vertex.GetParam(), vertex.m_entityId);
    m_clusteredVertexMap[vertex] = index;
    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
GeomPart::GeomPart(DRange3dCR range, GeometryList const& geometries) : m_range (range), m_facetCount(0), m_geometries(geometries)
    { 
    }                

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeomPart::IsCurved() const
    {
    for (auto& geometry : m_geometries)
        if (geometry->IsCurved())
            return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceList GeomPart::GetPolyfaces(IFacetOptionsR facetOptions, GeometryCR instance)
    {
    PolyfaceList polyfaces;
    for (auto& geometry : m_geometries) 
        {
        PolyfaceList thisPolyfaces = geometry->GetPolyfaces (facetOptions);

        if (!thisPolyfaces.empty())
            polyfaces.insert (polyfaces.end(), thisPolyfaces.begin(), thisPolyfaces.end());
        }

    for (auto& polyface : polyfaces)
        polyface.Transform(instance.GetTransform());

    return polyfaces;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StrokesList GeomPart::GetStrokes (IFacetOptionsR facetOptions, GeometryCR instance)
    {
    StrokesList strokes;

    for (auto& geometry : m_geometries) 
        {
        StrokesList   thisStrokes = geometry->GetStrokes(facetOptions);

        if (!thisStrokes.empty())
            strokes.insert (strokes.end(), thisStrokes.begin(), thisStrokes.end());
        }

    for (auto& stroke : strokes)
        stroke.Transform(instance.GetTransform());

    return strokes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t GeomPart::GetFacetCount(FacetCounter& counter, GeometryCR instance) const
    {
    if (0 == m_facetCount)
        for (auto& geometry : m_geometries) 
            m_facetCount += geometry->GetFacetCount(counter);
            
    return m_facetCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
Geometry::Geometry(TransformCR tf, DRange3dCR range, DgnElementId entityId, DisplayParamsR params, bool isCurved, DgnDbP db)
    : m_params(&params), m_transform(tf), m_tileRange(range), m_entityId(entityId), m_isCurved(isCurved), m_facetCount(0), m_hasTexture(nullptr != db && params.QueryTexture(db).IsValid())
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Geometry::GetFacetCount(IFacetOptionsR options) const
    {
    if (0 != m_facetCount)
        return m_facetCount;
    
    FacetCounter counter(options);
    return (m_facetCount = _GetFacetCount(counter));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
IFacetOptionsPtr Geometry::CreateFacetOptions(double chordTolerance, NormalMode normalMode) const
    {
    auto facetOptions = createTileFacetOptions(chordTolerance / m_transform.ColumnXMagnitude());
    bool normalsRequired = false;

    switch (normalMode)
        {
        case NormalMode::Always:    
            normalsRequired = true; 
            break;
        case NormalMode::CurvedSurfacesOnly:    
            normalsRequired = m_isCurved; 
            break;
        }

    facetOptions->SetNormalsRequired(normalsRequired);
    facetOptions->SetParamsRequired(HasTexture());

    return facetOptions;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceList Geometry::GetPolyfaces(double chordTolerance, NormalMode normalMode)
    {
    return _GetPolyfaces(*CreateFacetOptions(chordTolerance, normalMode));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Strokes::Transform(TransformCR transform)
    {
    for (auto& stroke : m_strokes)
        transform.Multiply (stroke, stroke);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceList PrimitiveGeometry::_GetPolyfaces(IFacetOptionsR facetOptions)
    {
    PolyfaceHeaderPtr polyface = m_geometry->GetAsPolyfaceHeader();
    
    if (polyface.IsValid())
        {
        if (!HasTexture())
            polyface->ClearParameters(false);

        BeAssertOnce(GetTransform().IsIdentity()); // Polyfaces are transformed during collection.
        return PolyfaceList (1, Polyface(*GetDisplayParamsPtr(), *polyface));
        }

    CurveVectorPtr      curveVector = m_geometry->GetAsCurveVector();

    if (curveVector.IsValid() && !curveVector->IsAnyRegionType())       // Non region curveVectors....
        return PolyfaceList();

    IPolyfaceConstructionPtr polyfaceBuilder = IPolyfaceConstruction::Create(facetOptions);

    ISolidPrimitivePtr  solidPrimitive = curveVector.IsNull() ? m_geometry->GetAsISolidPrimitive() : nullptr;
    MSBsplineSurfacePtr bsplineSurface = solidPrimitive.IsNull() && curveVector.IsNull() ? m_geometry->GetAsMSBsplineSurface() : nullptr;

    if (curveVector.IsValid())
        polyfaceBuilder->AddRegion(*curveVector);
    else if (solidPrimitive.IsValid())
        polyfaceBuilder->AddSolidPrimitive(*solidPrimitive);
    else if (bsplineSurface.IsValid())
        polyfaceBuilder->Add(*bsplineSurface);

    PolyfaceList    polyfaces;

    polyface = polyfaceBuilder->GetClientMeshPtr();
    if (polyface.IsValid())
        {
        polyface->Transform(GetTransform());
        polyfaces.push_back (Polyface(*GetDisplayParamsPtr(), *polyface));
        }

    return polyfaces;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StrokesList PrimitiveGeometry::_GetStrokes (IFacetOptionsR facetOptions)
    {
    CurveVectorPtr      curveVector = m_geometry->GetAsCurveVector();
    StrokesList         tileStrokes;

    if (curveVector.IsValid() && ! curveVector->IsAnyRegionType())
        {
        bvector<bvector<DPoint3d>>  strokePoints;

        collectCurveStrokes(strokePoints, *curveVector, facetOptions, GetTransform());

        if (!strokePoints.empty())
            tileStrokes.push_back(Strokes(*GetDisplayParamsPtr(), std::move(strokePoints)));
        }

    return tileStrokes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceList SolidKernelGeometry::_GetPolyfaces(IFacetOptionsR facetOptions)
    {
    PolyfaceList tilePolyfaces;
#if defined (BENTLEYCONFIG_PARASOLID)    
    // Cannot process the same solid entity simultaneously from multiple threads...
    BeMutexHolder lock(m_mutex);

    DRange3d entityRange = m_entity->GetEntityRange();
    if (entityRange.IsNull())
        return tilePolyfaces;

    double              rangeDiagonal = entityRange.DiagonalDistance();
    static double       s_minRangeRelTol = 1.0e-4;
    double              minChordTolerance = rangeDiagonal * s_minRangeRelTol;
    IFacetOptionsPtr    pFacetOptions = facetOptions.Clone();
    
    if (facetOptions.GetChordTolerance() < minChordTolerance)
        pFacetOptions->SetChordTolerance (minChordTolerance);

    pFacetOptions->SetParamsRequired (true); // Can't rely on HasTexture due to face attached material that may have texture.

    if (nullptr != m_entity->GetFaceMaterialAttachments())
        {
        bvector<PolyfaceHeaderPtr>  polyfaces;
        bvector<FaceAttachment>     params;

        if (!BRepUtil::FacetEntity(*m_entity, polyfaces, params, *pFacetOptions))
            return tilePolyfaces;

        GeometryParams baseParams;

        // Require valid category/subcategory for sub-category appearance color/material...
        baseParams.SetCategoryId(GetDisplayParams().GetCategoryId());
        baseParams.SetSubCategoryId(GetDisplayParams().GetSubCategoryId());

        for (size_t i=0; i<polyfaces.size(); i++)
            {
            auto&   polyface = polyfaces[i];

            if (polyface->HasFacets())
                {
                GeometryParams faceParams;

                params[i].ToGeometryParams(faceParams, baseParams);

#if defined(ELEMENT_TILE_FILL_COLOR_ONLY)
                DisplayParamsPtr displayParams = DisplayParams::Create (GetDisplayParams().GetFillColorDef(), faceParams);
#else
                DisplayParamsPtr displayParams = GetDisplayParamsPtr();
#endif

                tilePolyfaces.push_back (Polyface(*displayParams, *polyface));
                }
            }
        }
    else
        {
        auto polyface = BRepUtil::FacetEntity(*m_entity, *pFacetOptions);
    
        if (polyface.IsValid() && polyface->HasFacets())
            tilePolyfaces.push_back (Polyface(*GetDisplayParamsPtr(), *polyface));

        }

    if (!GetTransform().IsIdentity())
        for (auto& tilePolyface : tilePolyfaces)
            tilePolyface.m_polyface->Transform (GetTransform());

    return tilePolyfaces;

#else
    return tilePolyfaces;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryPtr Geometry::Create(TextStringR textString, TransformCR transform, DRange3dCR range, DgnElementId entityId, DisplayParamsR params, DgnDbP db)
    {
    return TextStringGeometry::Create(textString, transform, range, entityId, params, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryPtr Geometry::Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId entityId, DisplayParamsR params, bool isCurved, DgnDbP db)
    {
    return PrimitiveGeometry::Create(geometry, tf, range, entityId, params, isCurved, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryPtr Geometry::Create(IBRepEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId entityId, DisplayParamsR params, DgnDbP db)
    {
    return SolidKernelGeometry::Create(solid, tf, range, entityId, params, db);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryPtr Geometry::Create(GeomPartR part, TransformCR transform, DRange3dCR range, DgnElementId entityId, DisplayParamsR params, DgnDbP db)
    {
    return GeomPartInstanceGeometry::Create(part, transform, range, entityId, params, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Loader::Loader(TileR tile, TileTree::TileLoadStatePtr loads)
    : T_Super("", tile, loads, ""), m_filter(tile.GetElementRoot().GetFilter())
    {
    // NB: We must copy the filter, here on the main thread, because it may change while we're processing it...
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

    TilePolylineArgs polylineArgs;
    TileMeshArgs meshArgs;
    TileInstances tileInstances;
    Render::GraphicBuilderPtr graphic;

    if (InstancingOptions::AsInstances == s_instancingOptions)
        {
        for (auto const& part : geometry.Parts())
            {
            if (part->Instances().empty() || part->Meshes().empty())
                continue;

            for (auto const& mesh : part->Meshes())
                {
                bool haveMesh = !mesh->Triangles().empty();
                bool havePolyline = !haveMesh && !mesh->Polylines().empty();
                if (!haveMesh && !havePolyline)
                    continue;

                if (graphic.IsNull())
                    graphic = system._CreateGraphic(Graphic::CreateParams());

                auto subGraphic = graphic->CreateSubGraphic(Transform::FromIdentity());
                subGraphic->ActivateGraphicParams(mesh->GetDisplayParams().GetGraphicParams(), mesh->GetDisplayParams().GetGeometryParams());

                tileInstances.Init(part->Instances());
                if (haveMesh)
                    {
                    if (meshArgs.Init(*mesh, system, &root.GetDgnDb()))
                        subGraphic->AddInstancedTriMesh(meshArgs, tileInstances);
                    }
                else if (polylineArgs.Init(*mesh))
                    {
                    subGraphic->AddInstancedPolylines(polylineArgs, tileInstances);
                    }

                subGraphic->Close();
                graphic->AddSubGraphic(*subGraphic, Transform::FromIdentity(), mesh->GetDisplayParams().GetGraphicParams());
                }
            }
        }
    else if (InstancingOptions::AsSubGraphics == s_instancingOptions)
        {
        for (auto const& part : geometry.Parts())
            {
            if (part->Instances().empty() || part->Meshes().empty())
                continue;

            for (auto const& mesh : part->Meshes())
                {
                bool haveMesh = !mesh->Triangles().empty();
                bool havePolyline = !haveMesh && !mesh->Polylines().empty();
                if (!haveMesh && !havePolyline)
                    continue;

                if (graphic.IsNull())
                    graphic = system._CreateGraphic(Graphic::CreateParams());

                auto subGraphic = graphic->CreateSubGraphic(Transform::FromIdentity());
                subGraphic->ActivateGraphicParams(mesh->GetDisplayParams().GetGraphicParams(), mesh->GetDisplayParams().GetGeometryParams());

                if (haveMesh)
                    {
                    if (meshArgs.Init(*mesh, system, &root.GetDgnDb()))
                        subGraphic->AddTriMesh(meshArgs);
                    }
                else
                    {
                    polylineArgs.InitAndApply(*subGraphic, *mesh);
                    }

                subGraphic->Close();

                for (auto const& instance : part->Instances())
                    {
                    graphic->AddSubGraphic(*subGraphic, instance.GetTransform(), mesh->GetDisplayParams().GetGraphicParams());
                    }
                }
            }
        }
    else if (InstancingOptions::AsGraphics == s_instancingOptions)
        {
        for (auto const& part : geometry.Parts())
            {
            if (part->Instances().empty() || part->Meshes().empty())
                continue;

            for (auto const& mesh : part->Meshes())
                {
                bool haveMesh = !mesh->Triangles().empty();
                bool havePolyline = !haveMesh && !mesh->Polylines().empty();
                if (!haveMesh && !havePolyline)
                    continue;
                else if ((haveMesh && !meshArgs.Init(*mesh, system, &root.GetDgnDb())))
                    continue;
                else if ((havePolyline && !polylineArgs.Init(*mesh)))
                    continue;

                for (auto const& instance : part->Instances())
                    {
                    auto instanceGraphic = system._CreateGraphic(Graphic::CreateParams(nullptr, instance.GetTransform()));
                    instanceGraphic->ActivateGraphicParams(mesh->GetDisplayParams().GetGraphicParams(), mesh->GetDisplayParams().GetGeometryParams());
                    if (haveMesh)
                        instanceGraphic->AddTriMesh(meshArgs);
                    else
                        polylineArgs.Apply(*instanceGraphic);

                    instanceGraphic->Close();
                    tile.AddGraphic(*instanceGraphic);
                    }
                }
            }
        }
    else
        {
        for (auto const& part : geometry.Parts())
            {
            if (part->Instances().empty() || part->Meshes().empty())
                continue;

            for (auto const& mesh : part->Meshes())
                {
                bool haveMesh = !mesh->Triangles().empty();
                bool havePolyline = !haveMesh && !mesh->Polylines().empty();
                if (!haveMesh && !havePolyline)
                    continue;
                else if ((haveMesh && !meshArgs.Init(*mesh, system, &root.GetDgnDb())))
                    continue;
                else if ((havePolyline && !polylineArgs.Init(*mesh)))
                    continue;

                if (graphic.IsNull())
                    graphic = system._CreateGraphic(Graphic::CreateParams());

                Transform invTransform = Transform::FromIdentity();
                for (auto const& instance : part->Instances())
                    {
                    auto instanceGraphic = graphic->CreateSubGraphic(Transform::FromIdentity());
                    instanceGraphic->ActivateGraphicParams(mesh->GetDisplayParams().GetGraphicParams(), mesh->GetDisplayParams().GetGeometryParams());

                    // Transform the geometry in-place for each instance. This way we avoid the surprising overhead of qv_pushTransClip() while
                    // still keeping the scene's graphic list small by defining subgraphics
                    TransformCR instanceTransform = Transform::FromProduct(instance.GetTransform(), invTransform);
                    invTransform.InverseOf(instance.GetTransform());

                    if (haveMesh)
                        {
                        meshArgs.Transform(instanceTransform);
                        instanceGraphic->AddTriMesh(meshArgs);
                        }
                    else
                        {
                        polylineArgs.Transform(instanceTransform);
                        polylineArgs.Apply(*instanceGraphic);
                        }

                    instanceGraphic->Close();
                    graphic->AddSubGraphic(*instanceGraphic, Transform::FromIdentity(), mesh->GetDisplayParams().GetGraphicParams());
                    }

                // The mesh's vertices are const and reused...if we applied a transform, undo it.
                if (haveMesh && !invTransform.IsIdentity())
                    {
                    invTransform.InverseOf(invTransform);
                    meshArgs.Transform(invTransform);
                    }
                }
            }
        }

    bool addAsSubGraphics = true;
    for (auto const& mesh : geometry.Meshes())
        {
        bool haveMesh = !mesh->Triangles().empty();
        bool havePolyline = !haveMesh && !mesh->Polylines().empty();
        if (!haveMesh && !havePolyline)
            continue;

        if (graphic.IsNull())
            graphic = system._CreateGraphic(Graphic::CreateParams());

        Render::GraphicBuilderPtr thisGraphic = addAsSubGraphics ? graphic->CreateSubGraphic(Transform::FromIdentity()) : graphic;
        thisGraphic->ActivateGraphicParams(mesh->GetDisplayParams().GetGraphicParams(), mesh->GetDisplayParams().GetGeometryParams());

        if (haveMesh)
            {
            if (meshArgs.Init(*mesh, system, &root.GetDgnDb()))
                thisGraphic->AddTriMesh(meshArgs);
            }
        else
            {
            BeAssert(havePolyline);
            polylineArgs.InitAndApply(*thisGraphic, *mesh);
            }

        if (addAsSubGraphics)
            {
            thisGraphic->Close();
            graphic->AddSubGraphic(*thisGraphic, Transform::FromIdentity(), mesh->GetDisplayParams().GetGraphicParams());
            }

        addAsSubGraphics = true;
        }

#if defined(DRAW_EMPTY_RANGE_BOXES)
    if (graphic.IsNull())
        graphic = system._CreateGraphic(Graphic::CreateParams());
#endif

    // ###TODO: Doesn't handle case in which all graphics were instanced...
    if (graphic.IsValid())
        {
        if (root.WantDebugRanges())
            {
            ColorDef color = geometry.IsEmpty() ? ColorDef::Red() : tile.IsLeaf() ? ColorDef::DarkBlue() : ColorDef::DarkOrange();
            GraphicParams gfParams;
            gfParams.SetLineColor(color);
            gfParams.SetFillColor(ColorDef::Green());
            gfParams.SetWidth(0);
            gfParams.SetLinePixels(GraphicParams::LinePixels::Solid);

            Render::GraphicBuilderPtr rangeGraphic = addAsSubGraphics ? graphic->CreateSubGraphic(Transform::FromIdentity()) : graphic;
            rangeGraphic->ActivateGraphicParams(gfParams);
            rangeGraphic->AddRangeBox(tile.GetRange());
            if (addAsSubGraphics)
                {
                rangeGraphic->Close();
                graphic->AddSubGraphic(*rangeGraphic, Transform::FromIdentity(), gfParams);
                }
            }

        graphic->Close();
        tile.AddGraphic(*graphic);
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
Root::Root(GeometricModelR model, TransformCR transform, Render::SystemR system, ViewControllerCR view)
    : T_Super(model.GetDgnDb(), transform, "", &system), m_modelId(model.GetModelId()), m_name(model.GetName()),
    m_leafTolerance(s_minLeafTolerance), m_maxPointsPerTile(s_maxPointsPerTile), m_filter(view), m_is3d(model.Is3dModel()),
    m_debugRanges(ELEMENT_TILE_DEBUG_RANGE)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
RootPtr Root::Create(GeometricModelR model, Render::SystemR system, ViewControllerCR view)
    {
    DgnDb::VerifyClientThread();

    if (DgnDbStatus::Success != model.FillRangeIndex())
        return nullptr;

    DRange3d range;
    if (model.Is3dModel())
        {
        range = model.GetDgnDb().Units().GetProjectExtents();
        }
    else
        {
        // ###TODO_ELEMENT_TILE: What happens if user later adds geometry outside initial range?
        RangeAccumulator accum(range, model.Is2dModel());
        if (!accum.Accumulate(*model.GetRangeIndex()))
            return nullptr;
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

    RootPtr root = new Root(model, transform, system, view);
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
GeomPartPtr Root::FindOrInsertGeomPart(ISolidPrimitiveR prim, DRange3dCR range, DisplayParamsR displayParams, DgnElementId elemId) const
    {
    BeMutexHolder lock(m_mutex);
    return m_solidPrimitiveParts.FindOrInsert(prim, range, displayParams, elemId, GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
GeomPartPtr Root::SolidPrimitivePartMap::FindOrInsert(ISolidPrimitiveR prim, DRange3dCR range, DisplayParamsR displayParams, DgnElementId elemId, DgnDbR db)
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
    geomList.push_back(Geometry::Create(*geom, Transform::FromIdentity(), range, elemId, displayParams, true, &db));
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
    return !(*m_displayParams < *rhs.m_displayParams)
        && !(*rhs.m_displayParams < *m_displayParams)
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
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Filter::Filter(ViewControllerCR view)
    : m_categories(view.GetViewedCategories()), m_alwaysDrawn(view.GetAlwaysDrawn()), m_neverDrawn(view.GetNeverDrawn()), m_alwaysDrawnExclusive(view.IsAlwaysDrawnExclusive())
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Filter::AcceptElement(DgnElementId elem, DgnCategoryId cat) const
    {
    bool always = m_alwaysDrawn.end() != m_alwaysDrawn.find(elem);
    if (always || m_alwaysDrawnExclusive)
        return always;

    return m_neverDrawn.end() == m_neverDrawn.find(elem) && m_categories.end() != m_categories.find(cat);
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

        virtual Accept _CheckRangeTreeNode(RangeIndex::FBoxCR box, bool is3d) const override
            {
            return !ThresholdReached() && box.IntersectsWith(m_range) ? Accept::Yes : Accept::No;
            }

        virtual Stop _VisitRangeTreeEntry(RangeIndex::EntryCR entry) override
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
    return 512; // ###TODO: come up with a decent value, and account for device ppi
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
MeshList Tile::GenerateMeshes(GeometryOptionsCR options, GeometryList const& geometries, bool doRangeTest, LoadContextCR loadContext) const
    {
    static const double         s_vertexClusterThresholdPixels = 5.0;
    static const size_t         s_decimatePolyfacePointCount = 100;

    DgnDbR  db = GetElementRoot().GetDgnDb();
    auto    normalMode = options.m_normalMode;
    bool    twoSidedTriangles = options.WantTwoSidedTriangles();
    bool    doSurfacesOnly = options.WantSurfacesOnly();

    double  tolerance = GetTolerance();
    double  vertexTolerance = tolerance * s_vertexToleranceRatio;
    double  facetAreaTolerance = tolerance * tolerance * s_facetAreaToleranceRatio;

    // Convert to meshes
    bmap<MeshMergeKey, MeshBuilderPtr> builderMap;
    size_t      geometryCount = 0;
    DRange3d    myTileRange = GetTileRange();

    MeshList meshes;

    for (auto& geom : geometries)
        {
        if (loadContext.WasAborted())
            return meshes;

        DRange3dCR  geomRange = geom->GetTileRange();
        double      rangePixels = geomRange.DiagonalDistance() / tolerance;

        if (rangePixels < s_minRangeBoxSize)
            continue;   // ###TODO: -- Produce an artifact from optimized bounding box to approximate from range.

        auto        polyfaces = geom->GetPolyfaces(tolerance, normalMode);
        bool        isContained = !doRangeTest || geomRange.IsContained(myTileRange);
        bool        maxGeometryCountExceeded = (++geometryCount > s_maxGeometryIdCount);

        for (auto& tilePolyface : polyfaces)
            {
            DisplayParamsPtr        displayParams = tilePolyface.m_displayParams;
            PolyfaceHeaderPtr       polyface = tilePolyface.m_polyface;
            bool                    hasTexture = displayParams.IsValid() && displayParams->QueryTexture(&db).IsValid();  // Can't rely on geom.HasTexture - this may come from a face attachment to a B-Rep.

            if (0 == polyface->GetPointCount())
                continue;

            MeshMergeKey key(*displayParams, polyface.IsValid() && nullptr != polyface->GetNormalIndexCP(), polyface.IsValid());

            MeshBuilderPtr meshBuilder;
            auto found = builderMap.find(key);
            if (builderMap.end() != found)
                meshBuilder = found->second;
            else
                builderMap[key] = meshBuilder = MeshBuilder::Create(*displayParams, vertexTolerance, facetAreaTolerance);

            if (polyface.IsValid())
                {
                // Decimate if the range of the geometry is small in the tile OR we are not in a leaf and we have geometry originating from polyface with many points (railings from Penn state building).
                // A polyface with many points is likely a tesselation from an outside source.
                bool        doDecimate          = !m_isLeaf && geom->DoDecimate() && polyface->GetPointCount() > s_decimatePolyfacePointCount;
                bool        doVertexCluster     = !doDecimate && geom->DoVertexCluster() && rangePixels < s_vertexClusterThresholdPixels;

                if (doDecimate)
                    polyface->DecimateByEdgeCollapse (tolerance, 0.0);

                for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*polyface); visitor->AdvanceToNextFace(); /**/)
                    {
                    if (isContained || myTileRange.IntersectsWith(DRange3d::From(visitor->GetPointCP(), static_cast<int32_t>(visitor->Point().size()))))
                        {
                        DgnElementId elemId;
                        if (!maxGeometryCountExceeded)
                            elemId = geom->GetEntityId();

                        meshBuilder->AddTriangle (*visitor, displayParams->GetMaterialId(), &db, elemId, doVertexCluster, twoSidedTriangles, hasTexture);
                        }
                    }
                }
            }

        if (!doSurfacesOnly)
            {
            auto                tileStrokesArray = geom->GetStrokes(*geom->CreateFacetOptions (tolerance, NormalMode::Never));
        
            for (auto& tileStrokes : tileStrokesArray)
                {
                if (loadContext.WasAborted())
                    return meshes;

                DisplayParamsPtr displayParams = tileStrokes.m_displayParams;
                MeshMergeKey key(*displayParams, false, false);

                MeshBuilderPtr meshBuilder;
                auto found = builderMap.find(key);
                if (builderMap.end() != found)
                    meshBuilder = found->second;
                else
                    builderMap[key] = meshBuilder = MeshBuilder::Create(*displayParams, vertexTolerance, facetAreaTolerance);

                DgnElementId elemId;
                if (geometryCount < s_maxGeometryIdCount)
                    elemId = geom->GetEntityId();

                for (auto& strokePoints : tileStrokes.m_strokes)
                    meshBuilder->AddPolyline (strokePoints, elemId, rangePixels < s_vertexClusterThresholdPixels);
                }
            }
        }

    for (auto& builder : builderMap)
        if (!builder.second->GetMesh()->IsEmpty())
            meshes.push_back (builder.second->GetMesh());

    return meshes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementTileTree::GeometryCollection Tile::GenerateGeometry(GeometryOptionsCR options, LoadContextCR context)
    {
    ElementTileTree::GeometryCollection geom;

    auto const& root = GetElementRoot();

#if defined(ELEMENT_TILE_CHECK_FACET_COUNTS)
    // Always collect geometry at the target leaf tolerance.
    // If we exceed our leaf threshold, we'll keep the geometry but adjust this tile's target tolerance
    bool leafThresholdExceeded = false;
    GeometryList geometries = CollectGeometry(&leafThresholdExceeded, root.GetLeafTolerance(), options.WantSurfacesOnly(), m_isLeaf ? 0 : root.GetMaxPointsPerTile(), context);

    if (context.WasAborted())
        return geom;

    if (!m_isLeaf && !leafThresholdExceeded)
        SetIsLeaf();

    if (geometries.empty())
        return geom;

    if (m_isLeaf)
        m_tolerance = root.GetLeafTolerance();
    else
        adjustGeometryTolerance(geometries, m_tolerance);
#elif defined(ELEMENT_TILE_TRUNCATE_PLANAR)
    // Always collect geometry at the target leaf tolerance.
    // If we find no curved geometry, there's no point in creating child nodes.
    // Otherwise, keep all the geometry that is large enough to display at this tile's tolerance and facet to that tolerance
    GeometryList geometries = CollectGeometry(nullptr, root.GetLeafTolerance(), options.WantSurfacesOnly(), m_isLeaf ? 0 : root.GetMaxPointsPerTile(), context);
    if (context.WasAborted())
        return geom;

    bool anyCurved = false;
    for (auto const& geometry : geometries)
        {
        if (anyCurved = geometry->IsCurved())
            break;
        }

    if (!anyCurved)
        {
        m_tolerance = root.GetLeafTolerance();
        SetIsLeaf();
        }
    else
        {
        adjustGeometryTolerance(geometries, m_tolerance);
        }
#else
    GeometryList geometries = CollectGeometry(nullptr, m_tolerance, options.WantSurfacesOnly(), root.GetMaxPointsPerTile(), context);
#endif

    return CreateGeometryCollection(geometries, options, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementTileTree::GeometryCollection Tile::CreateGeometryCollection(GeometryList const& geometries, GeometryOptionsCR options, LoadContextCR context) const
    {
    ElementTileTree::GeometryCollection collection;
    
    GeometryList                        uninstancedGeometry;
    bmap<GeomPartCP,MeshPartPtr> partMap;

    // Extract instances first...
    for (auto const& geom : geometries)
        {
        if (context.WasAborted())
            return collection;

        auto const& part = geom->GetPart();
        if (part.IsValid())
            {
            MeshPartPtr meshPart;
            auto found = partMap.find(part.get());

            if (partMap.end() == found)
                {
                MeshList partMeshes = GenerateMeshes(options, part->GetGeometries(), false, context);
                if (partMeshes.empty())
                    continue;

                collection.Parts().push_back(meshPart = MeshPart::Create(std::move(partMeshes)));
                partMap.Insert(part.get(), meshPart);
                }
            else
                {
                meshPart = found->second;
                }

            meshPart->AddInstance(MeshInstance(geom->GetEntityId(), geom->GetTransform()));
            }
        else
            {
            uninstancedGeometry.push_back(geom);
            }
        }

    auto& meshes = collection.Meshes();
    MeshList uninstancedMeshes = GenerateMeshes(options, uninstancedGeometry, true, context);
    meshes.insert(meshes.end(), uninstancedMeshes.begin(), uninstancedMeshes.end());

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
GeometryList Tile::CollectGeometry(bool* leafThresholdExceeded, double tolerance, bool surfacesOnly, size_t leafCountThreshold, LoadContextCR loadContext)
    {
    auto& root = GetElementRoot();
    auto is2d = root.Is2d();
    IFacetOptionsPtr facetOptions = createTileFacetOptions(tolerance);

    Transform transformFromDgn;
    transformFromDgn.InverseOf(root.GetLocation());

    GeometryList geometries;

    if (loadContext.WasAborted())
        return geometries;

    TileGeometryProcessor processor(geometries, root, GetDgnRange(), *facetOptions, transformFromDgn, leafThresholdExceeded, tolerance, surfacesOnly, leafCountThreshold, is2d);

    GeometryProcessorContext context(processor, root, loadContext);
    processor.OutputGraphics(context);

    return geometries;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d Tile::ComputeChildRange(TileR child) const
    {
    // Each dimension of the relative ID is 0 or 1, indicating which bisection of the range to take
    TileTree::OctTree::TileId relativeId = child.GetRelativeTileId();
    BeAssert(2 > relativeId.m_i && 2 > relativeId.m_j && 2 > relativeId.m_k);

    DRange3d range = bisectRange(GetRange(), 0 == relativeId.m_i);
    range = bisectRange(range, 0 == relativeId.m_j);
    range = bisectRange(range, 0 == relativeId.m_k);

    return range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometryProcessor::ProcessElement(ViewContextR context, DgnElementId elemId, DRange3dCR dgnRange)
    {
    try
        {
#if defined(ELEMENT_TILE_REGENERATION)
        TileModelDelta::ElementState*    elementState = nullptr;
        if (nullptr != m_modelDelta)
            {
            if (nullptr == (elementState = m_modelDelta->GetElementState(elemId)))
                {
                BeAssert (false && "Unexpected Element");
                return;
                }
            if (!m_modelDelta->DoPublish(elemId))
                {
                DRange3d intersection = DRange3d::FromIntersection (dgnRange, m_range, true);

                if (intersection.IsNull())
                    return;

                m_leafCount += (size_t) ((double) elementState->GetFacetCount() * intersection.DiagonalDistance() / dgnRange.DiagonalDistance());
                *m_leafThresholdExceeded = (m_leafCount > m_leafCountThreshold);

                return;
                }
            }
#endif

        m_geometryListBuilder.Clear();
        m_geometryListBuilder.SetElementId(elemId);
        context.VisitElement(elemId, false);

        for (auto& geom : m_geometryListBuilder.GetGeometries())
            PushGeometry(*geom);

#if defined(ELEMENT_TILE_REGENERATION)
        if (nullptr != elementState && 0 == elementState->GetFacetCount())    
            for (auto& geom : m_geometryListBuilder.GetGeometries())
                elementState->SetFacetCount(elementState->GetFacetCount() + geom->GetFacetCount(*m_targetFacetOptions));
#endif
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

    return m_geometryListBuilder.AddCurveVector(curves, filled, *CreateDefaultDisplayParams(gf), gf.GetLocalToWorldTransform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessSolidPrimitive(ISolidPrimitiveCR prim, SimplifyGraphic& gf) 
    {
    DisplayParamsPtr displayParams = CreateDefaultDisplayParams(gf);
    if (s_cacheInstances)
        {
        DRange3d range, thisTileRange;
        Transform tf = Transform::FromProduct(GetTransformFromDgn(), gf.GetLocalToWorldTransform());
        prim.GetRange(range);
        tf.Multiply(thisTileRange, range);
        if (thisTileRange.IsContained(m_tileRange))
            {
            ISolidPrimitivePtr clone = prim.Clone();
            GeomPartPtr geomPart = m_root.FindOrInsertGeomPart(*clone, range, *displayParams, GetCurrentElementId());
            AddElementGeometry(*Geometry::Create(*geomPart, tf, thisTileRange, GetCurrentElementId(), *displayParams, &GetDgnDb()));
            return true;
            }
        }

    return m_geometryListBuilder.AddSolidPrimitive(prim, *displayParams, gf.GetLocalToWorldTransform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessSurface(MSBsplineSurfaceCR surface, SimplifyGraphic& gf) 
    {
    return m_geometryListBuilder.AddSurface(surface, *CreateDefaultDisplayParams(gf), gf.GetLocalToWorldTransform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessPolyface(PolyfaceQueryCR polyface, bool filled, SimplifyGraphic& gf) 
    {
    return m_geometryListBuilder.AddPolyface(polyface, filled, *CreateDefaultDisplayParams(gf), gf.GetLocalToWorldTransform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessBody(IBRepEntityCR solid, SimplifyGraphic& gf) 
    {
    return m_geometryListBuilder.AddBody(solid, *CreateDefaultDisplayParams(gf), gf.GetLocalToWorldTransform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessTextString(TextStringCR textString, SimplifyGraphic& gf) 
    {
    return m_geometryListBuilder.AddTextString(textString, *CreateDisplayParams(gf, true /*ignore lighting*/), gf.GetLocalToWorldTransform());
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
    if (BelowMinRange(geom.GetTileRange()))
        return;

    if (nullptr != m_leafThresholdExceeded && !(*m_leafThresholdExceeded))
        {
        DRange3d intersection = DRange3d::FromIntersection (geom.GetTileRange(), m_tileRange, true);

        if (intersection.IsNull())
            return;

        m_leafCount += (size_t) ((double) geom.GetFacetCount(*m_targetFacetOptions) * intersection.DiagonalDistance() / geom.GetTileRange().DiagonalDistance());
        *m_leafThresholdExceeded = (m_leafCount > m_leafCountThreshold);
        }
        
    m_geometries.push_back(&geom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometryProcessor::AddGeomPart (Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, GeometryParamsR geomParams, GraphicParamsR graphicParams, ViewContextR viewContext)
    {
    Transform               partToWorld = Transform::FromProduct(graphic.GetLocalToWorldTransform(), subToGraphic);
    DisplayParamsPtr        displayParams = DisplayParams::Create(graphicParams, geomParams);
    DRange3d                range;

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

        m_root.AddGeomPart (partId, *(tileGeomPart = GeomPart::Create(geomPart->GetBoundingBox(), m_geometryListBuilder.GetGeometries())));

        m_geometryListBuilder.SetGeometryList(saveCurrGeometries);
        m_geometryListBuilder.SetTransform(builderTransform);
        }

    Transform   tf = Transform::FromProduct(GetTransformFromDgn(), partToWorld);
    
    tf.Multiply(range, tileGeomPart->GetRange());
    AddElementGeometry(*Geometry::Create(*tileGeomPart, tf, range, GetCurrentElementId(), *displayParams, &GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryProcessor::Result TileGeometryProcessor::OutputGraphics(GeometryProcessorContext& context)
    {
    GeometryCollector collector(m_range, *this, context);
    auto status = collector.Collect();
    if (Result::Aborted == status)
        {
        m_geometries.clear();
        }
#if defined(ELEMENT_TILE_BATCHIDS)
    else if (Result::Success == status)
        {
        // We sort by size in order to ensure the largest geometries are assigned batch IDs
        // If the number of geometries does not exceed the max number of batch IDs, they will all get batch IDs so sorting is unnecessary
        if (m_geometries.size() > s_maxGeometryIdCount)
            {
            std::sort(m_geometries.begin(), m_geometries.end(), [&](GeometryPtr const& lhs, GeometryPtr const& rhs)
                {
                DRange3d lhsRange, rhsRange;
                lhsRange.IntersectionOf(lhs->GetTileRange(), m_range);
                rhsRange.IntersectionOf(rhs->GetTileRange(), m_range);
                return lhsRange.DiagonalDistance() < rhsRange.DiagonalDistance();
                });
            }
        }
#endif

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr GeometryProcessorContext::_AddSubGraphic(Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, GeometryParamsR geomParams)
    {
    if (s_cacheInstances)
        {
        GraphicParams graphicParams;
        _CookGeometryParams(geomParams, graphicParams);

        m_processor.AddGeomPart(graphic, partId, subToGraphic, geomParams, graphicParams, *this);
        }
    else
        {
        return T_Super::_AddSubGraphic(graphic, partId, subToGraphic, geomParams);
        }
    return nullptr;
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
    m_processor.GetRoot().GetDbMutex().Enter();
    StatusInt status = ERROR;
    auto& stmt = *m_statement;
    stmt.BindInt64(1, static_cast<int64_t>(elementId.GetValueUnchecked()));

    if (BeSQLite::BE_SQLITE_ROW == stmt.Step() && !IsValueNull(1))
        {
        auto geomSrcPtr = m_is3dView ? GeometrySelector3d::ExtractGeometrySource(stmt, GetDgnDb()) : GeometrySelector2d::ExtractGeometrySource(stmt, GetDgnDb());

        stmt.Reset();
        m_processor.GetRoot().GetDbMutex().Leave();

        if (nullptr != geomSrcPtr)
            status = VisitGeometry(*geomSrcPtr);
        }
    else
        {
        stmt.Reset();
        m_processor.GetRoot().GetDbMutex().Leave();
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryListBuilder::AddGeometry(IGeometryR geom, bool isCurved, DisplayParamsR displayParams, TransformCR transform)
    {
    DRange3d range;
    if (!geom.TryGetRange(range))
        return false;

    auto tf = m_haveTransform ? Transform::FromProduct(m_transform, transform) : transform;
    tf.Multiply(range, range);

    return AddGeometry(geom, isCurved, displayParams, tf, range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryListBuilder::AddGeometry(IGeometryR geom, bool isCurved, DisplayParamsR displayParams, TransformCR transform, DRange3dCR range)
    {
    GeometryPtr geometry = Geometry::Create(geom, transform, range, GetElementId(), displayParams, isCurved, GetDgnDb());
    if (geometry.IsNull())
        return false;

    m_geometries.push_back(geometry);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryListBuilder::AddCurveVector(CurveVectorCR curves, bool filled, DisplayParamsR displayParams, TransformCR transform)
    {
    if (m_surfacesOnly && !curves.IsAnyRegionType())
        return true;    // ignore...

    bool isCurved = curves.ContainsNonLinearPrimitive();
    CurveVectorPtr clone = curves.Clone();
    IGeometryPtr geom = IGeometry::Create(clone);
    return AddGeometry(*geom, isCurved, displayParams, transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryListBuilder::AddSolidPrimitive(ISolidPrimitiveCR primitive, DisplayParamsR displayParams, TransformCR transform)
    {
    bool isCurved = primitive.HasCurvedFaceOrEdge();
    ISolidPrimitivePtr clone = primitive.Clone();
    IGeometryPtr geom = IGeometry::Create(clone);
    return AddGeometry(*geom, isCurved, displayParams, transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryListBuilder::AddSurface(MSBsplineSurfaceCR surface, DisplayParamsR displayParams, TransformCR transform)
    {
    MSBsplineSurfacePtr clone = MSBsplineSurface::CreatePtr();
    clone->CopyFrom(surface);
    IGeometryPtr geom = IGeometry::Create(clone);

    bool isCurved = (clone->GetUOrder() > 2 || clone->GetVOrder() > 2);
    return AddGeometry(*geom, isCurved, displayParams, transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryListBuilder::AddPolyface(PolyfaceQueryCR polyface, bool filled, DisplayParamsR displayParams, TransformCR transform)
    {
    PolyfaceHeaderPtr clone = polyface.Clone();
    if (!clone->IsTriangulated())
        clone->Triangulate();

    if (m_haveTransform)
        clone->Transform(Transform::FromProduct(m_transform, transform));
    else if (!transform.IsIdentity())
        clone->Transform(transform);

    DRange3d range = clone->PointRange();
    IGeometryPtr geom = IGeometry::Create(clone);
    AddGeometry(*geom, false, displayParams, Transform::FromIdentity(), range);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryListBuilder::AddBody(IBRepEntityCR body, DisplayParamsR displayParams, TransformCR transform)
    {
    IBRepEntityPtr clone = const_cast<IBRepEntityP>(&body);

    DRange3d range = clone->GetEntityRange();
    Transform tf = m_haveTransform ? Transform::FromProduct(m_transform, transform) : transform;
    tf.Multiply(range, range);

    m_geometries.push_back(Geometry::Create(*clone, tf, range, GetElementId(), displayParams, GetDgnDb()));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryListBuilder::AddTextString(TextStringCR textString, DisplayParamsR displayParams, TransformCR transform)
    {
    if (m_surfacesOnly)
        return true;

    static BeMutex s_tempFontMutex;
    BeMutexHolder lock(s_tempFontMutex);    // Temporary - until we resolve the font threading issues.

    TextStringPtr clone = textString.Clone();
    Transform tf = m_haveTransform ? Transform::FromProduct(m_transform, transform) : transform;

    DRange2d range2d = clone->GetRange();
    DRange3d range = DRange3d::From(range2d.low.x, range2d.low.y, 0.0, range2d.high.x, range2d.high.y, 0.0);
    Transform::FromProduct(tf, clone->ComputeTransform()).Multiply(range, range);

    m_geometries.push_back(Geometry::Create(*clone, tf, range, GetElementId(), displayParams, GetDgnDb()));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
MeshList GeometryListBuilder::ToMeshes(GeometryOptionsCR options, double tolerance) const
    {
    MeshList meshes;
    if (m_geometries.empty())
        return meshes;

    double vertexTolerance = tolerance * s_vertexToleranceRatio;
    double facetAreaTolerance = tolerance * tolerance * s_facetAreaToleranceRatio;

    bmap<MeshMergeKey, MeshBuilderPtr> builderMap;
    for (auto const& geom : m_geometries)
        {
        auto polyfaces = geom->GetPolyfaces(tolerance, options.m_normalMode);
        for (auto const& tilePolyface : polyfaces)
            {
            PolyfaceHeaderPtr polyface = tilePolyface.m_polyface;
            if (polyface.IsNull() || 0 == polyface->GetPointCount())
                continue;

            DisplayParamsPtr displayParams = tilePolyface.m_displayParams;
            bool hasTexture = displayParams.IsValid() && displayParams->QueryTexture(GetDgnDb()).IsValid();

            MeshMergeKey key(*displayParams, nullptr != polyface->GetNormalIndexCP(), true);

            MeshBuilderPtr meshBuilder;
            auto found = builderMap.find(key);
            if (builderMap.end() != found)
                meshBuilder = found->second;
            else
                builderMap[key] = meshBuilder = MeshBuilder::Create(*displayParams, vertexTolerance, facetAreaTolerance);

            for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*polyface); visitor->AdvanceToNextFace(); /**/)
                meshBuilder->AddTriangle(*visitor, displayParams->GetMaterialId(), GetDgnDb(), geom->GetEntityId(), false, options.WantTwoSidedTriangles(), hasTexture);
            }

        if (!options.WantSurfacesOnly())
            {
            auto tileStrokesArray = geom->GetStrokes(*geom->CreateFacetOptions(tolerance, NormalMode::Never));
            for (auto& tileStrokes : tileStrokesArray)
                {
                DisplayParamsPtr displayParams = tileStrokes.m_displayParams;
                MeshMergeKey key(*displayParams, false, false);

                MeshBuilderPtr meshBuilder;
                auto found = builderMap.find(key);
                if (builderMap.end() != found)
                    meshBuilder = found->second;
                else
                    builderMap[key] = meshBuilder = MeshBuilder::Create(*displayParams, vertexTolerance, facetAreaTolerance);

                for (auto& strokePoints : tileStrokes.m_strokes)
                    meshBuilder->AddPolyline(strokePoints, geom->GetEntityId(), false);
                }
            }
        }

    for (auto& builder : builderMap)
        if (!builder.second->GetMesh()->IsEmpty())
            meshes.push_back(builder.second->GetMesh());

    return meshes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::SaveToGraphic(Render::GraphicBuilderR gf, Render::System const& system, GeometryOptionsCR options, double tolerance) const
    {
    TileMeshArgs meshArgs;
    TilePolylineArgs polylineArgs;

    bool asSubGraphic = true;
    MeshList meshes = ToMeshes(options, tolerance);
    for (auto const& mesh : meshes)
        {
        bool haveMesh = !mesh->Triangles().empty();
        bool havePolyline = !haveMesh && !mesh->Polylines().empty();
        if (!haveMesh && !havePolyline)
            continue;

        Render::GraphicBuilderPtr subGf = asSubGraphic ? gf.CreateSubGraphic(Transform::FromIdentity()) : &gf;
        subGf->ActivateGraphicParams(mesh->GetDisplayParams().GetGraphicParams(), mesh->GetDisplayParams().GetGeometryParams());

        if (havePolyline)
            polylineArgs.InitAndApply(*subGf, *mesh);
        else if (meshArgs.Init(*mesh, system, GetDgnDb()))
            subGf->AddTriMesh(meshArgs);

        if (asSubGraphic)
            {
            subGf->Close();
            gf.AddSubGraphic(*subGf, Transform::FromIdentity(), mesh->GetDisplayParams().GetGraphicParams());
            }

        asSubGraphic = true;
        }
    }

#if defined (BENTLEYCONFIG_PARASOLID) 
// ###TODO: ugh.
static ThreadedLocalParasolidHandlerStorageMark s_tempParasolidThreadedHandlerStorageMark;
#endif


/*-------------------------------------------------------------------------------------+                                                                                           
|
|     $Source: DgnCore/MeshTile.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <folly/BeFolly.h>
#include <DgnPlatform/RangeIndex.h>
#if defined (BENTLEYCONFIG_PARASOLID) 
#include <DgnPlatform/DgnBRep/PSolidUtil.h>
#endif

USING_NAMESPACE_BENTLEY_RENDER

BEGIN_UNNAMED_NAMESPACE

constexpr double s_half2dDepthRange = 10.0;
// unused - static size_t s_maxFacetDensity;

#if defined (BENTLEYCONFIG_PARASOLID) 

// The ThreadLocalParasolidHandlerStorageMark sets up the local storage that will be used 
// by all threads.

typedef RefCountedPtr <struct ThreadedParasolidErrorHandlerInnerMark>       ThreadedParasolidErrorHandlerInnerMarkPtr;

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


static ITileGenerationProgressMonitor   s_defaultProgressMeter;

static const double s_minRangeBoxSize    = 1.5;     // Threshold below which we consider geometry/element too small to contribute to tile mesh
static const size_t s_maxGeometryIdCount = 0xffff;  // Max batch table ID - 16-bit unsigned integers
static const double s_minToleranceRatio  = 256.0;   // Nominally the screen size of a tile.  Increasing generally increases performance (fewer draw calls) at expense of higher load times.

END_UNNAMED_NAMESPACE

#define COMPARE_VALUES_TOLERANCE(val0, val1, tol)   if (val0 < val1 - tol) return true; if (val0 > val1 + tol) return false;
#define COMPARE_VALUES(val0, val1) if (val0 < val1) { return true; } if (val0 > val1) { return false; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerationCache::TileGenerationCache(Options options) : m_range(DRange3d::NullRange()), m_options(options),
    m_dbMutex(BeSQLite::BeDbMutex::MutexType::Recursive)
    {
    // Caller will populate...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGenerationCache::AddCachedGeometry(DgnElementId elementId, TileGeometryList&& geometry) const
    {
    if (WantCacheGeometry())
        {
        BeMutexHolder lock(m_mutex);
        m_geometry.Insert(elementId, geometry);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGenerationCache::GetCachedGeometry(TileGeometryList& geometry, DgnElementId elementId) const
    {
    if (WantCacheGeometry())
        {
        BeMutexHolder lock(m_mutex);
        auto iter = m_geometry.find(elementId);
        if (m_geometry.end() != iter)
            {
            if (geometry.empty())
                geometry = iter->second;
            else
                geometry.insert(geometry.end(), iter->second.begin(), iter->second.end());

            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
GeometrySourceCP TileGenerationCache::AddCachedGeometrySource(std::unique_ptr<GeometrySource>& source, DgnElementId elemId) const
    {
    if (!WantCacheGeometrySources())
        return source.get();

    BeMutexHolder lock(m_mutex);

    // May already exist in cache...if so we've moved from it and it will be destroyed...otherwise it's now owned by cache
    m_geometrySources.insert(GeometrySourceMap::value_type(elemId, std::move(source)));

    // Either way, we know an now exists in cache for this element
    auto existing = GetCachedGeometrySource(elemId);
    BeAssert(nullptr != existing);
    return existing;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
GeometrySourceCP TileGenerationCache::GetCachedGeometrySource(DgnElementId elemId) const
    {
    if (!WantCacheGeometrySources())
        return nullptr;

    BeMutexHolder lock(m_mutex);
    auto iter = m_geometrySources.find(elemId);
    return m_geometrySources.end() != iter ? iter->second.get() : nullptr;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerationCache::~TileGenerationCache()
    {
    //
    }


//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
struct RangeAccumulator : RangeIndex::Traverser
{
    DRange3dR       m_range;
    bool            m_is2d;

    RangeAccumulator(DRange3dR range, bool is2d) : m_range(range), m_is2d(is2d) { m_range = DRange3d::NullRange(); }

    bool _AbortOnWriteRequest() const override { return true; }
    RangeIndex::Traverser::Accept _CheckRangeTreeNode(RangeIndex::FBoxCR, bool) const override { return RangeIndex::Traverser::Accept::Yes; }
    Stop _VisitRangeTreeEntry(RangeIndex::EntryCR entry) override
        {
        m_range.Extend(entry.m_range.ToRange3d());
        return Stop::No;
        }

    TileGeneratorStatus Accumulate(RangeIndex::Tree& tree)
        {
        if (Stop::Yes == tree.Traverse(*this))
            return TileGeneratorStatus::Aborted;
        else if (m_range.IsNull())
            return TileGeneratorStatus::NoGeometry;

        if (m_is2d)
            {
            BeAssert(m_range.low.z == m_range.high.z == 0.0);
            m_range.low.z = -s_half2dDepthRange*2;  // times 2 so we don't stick geometry right on the boundary...
            m_range.high.z = s_half2dDepthRange*2;
            }

        return TileGeneratorStatus::Success;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeneratorStatus TileGenerationCache::Populate(DgnDbR db, DgnModelR model)       
    {
    m_model = &model;
    auto geomModel = model.ToGeometricModelP();
    if (nullptr == geomModel || DgnDbStatus::Success != geomModel->FillRangeIndex())
        return TileGeneratorStatus::NoGeometry;

    RangeAccumulator accum(m_range, model.Is2dModel());
    return accum.Accumulate(*geomModel->GetRangeIndex());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureCPtr TileDisplayParams::QueryTexture(DgnDbR db) const
    {
    DgnMaterialCPtr material = DgnMaterial::Get(db, m_materialId);
    if (!material.IsValid())
        return nullptr;

    auto& mat = material->GetRenderingAsset();
    auto texMap = mat.GetPatternMap();
    DgnTextureId texId;
    if (!texMap.IsValid() || !(texId = texMap.GetTextureId()).IsValid())
        return nullptr;

    return DgnTexture::Get(db, texId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileDisplayParams::TileDisplayParams(GraphicParamsCP graphicParams, GeometryParamsCP geometryParams, bool ignoreLighting, bool useLineParams) :
                   m_color(0x00ffffff), m_ignoreLighting (ignoreLighting), m_rasterWidth(0), m_linePixels(0)
    {
    if (nullptr != geometryParams)
        {
        m_categoryId = geometryParams->GetCategoryId();
        m_subCategoryId = geometryParams->GetSubCategoryId();
        m_materialId = geometryParams->GetMaterialId();
        m_class = geometryParams->GetGeometryClass();
        m_isColorFromBackground = geometryParams->IsFillColorFromViewBackground();
        }
    if (nullptr != graphicParams)
        {
        if (useLineParams)
            {
            m_color            = graphicParams->GetLineColor().GetValue();
            m_rasterWidth      = graphicParams->GetWidth();
            m_linePixels       = graphicParams->GetLinePixels();
            }
        else
            {
            m_color = graphicParams->GetFillColor().GetValue(); 
            if (nullptr != graphicParams->GetGradientSymb())
                {
                m_gradient = GradientSymb::Create();
                m_gradient->CopyFrom(*graphicParams->GetGradientSymb());
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileDisplayParams::IsLessThan(TileDisplayParams const& rhs, bool compareColor) const
    {
    if (m_ignoreLighting != rhs.m_ignoreLighting)
        return m_ignoreLighting;

    if (m_isColorFromBackground != rhs.m_isColorFromBackground)
        return m_isColorFromBackground != rhs.m_isColorFromBackground;

    if (m_linePixels != rhs.m_linePixels)
        return m_linePixels < rhs.m_linePixels;                                                                                        

    if (m_gradient.get() != rhs.m_gradient.get())
        {
        if (!m_gradient.IsValid() || !rhs.m_gradient.IsValid() || ! (*m_gradient == *rhs.m_gradient))
            return m_gradient.get() < rhs.m_gradient.get();
        }

    if (m_color != rhs.m_color)
        {
        if (compareColor)
            return m_color < rhs.m_color;

        // cannot batch translucent and opaque meshes
        ColorDef lhsColor(m_color), rhsColor(rhs.m_color);
        bool lhsHasAlpha = 0 != lhsColor.GetAlpha(),
             rhsHasAlpha = 0 != rhsColor.GetAlpha();

        if (lhsHasAlpha != rhsHasAlpha)
            return lhsHasAlpha;

        }

    if (m_rasterWidth != rhs.m_rasterWidth)
        return m_rasterWidth < rhs.m_rasterWidth;

    if (m_materialId.GetValueUnchecked() != rhs.m_materialId.GetValueUnchecked())
        return m_materialId.GetValueUnchecked() < rhs.m_materialId.GetValueUnchecked();

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
ImageSource TileTextureImage::Load(TileDisplayParamsCR params, DgnDbR db)
    {
    DgnTextureCPtr tex = params.QueryTexture(db);
    return tex.IsValid() ? tex->GetImageSource() : ImageSource();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileTextureImagePtr TileTextureImage::Create(GradientSymbCR gradient)
    {
    static const size_t     s_size = 256;

    return TileTextureImage::Create(Render::ImageSource (gradient.GetImage(s_size, s_size), Render::ImageSource::Format::Png), false);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileDisplayParams::ResolveTextureImage(DgnDbR db) const
    {
    if (m_textureImage.IsValid())
        return;

    if (m_gradient.IsValid())
        {
        m_textureImage = TileTextureImage::Create(*m_gradient);
        return;
        }

    ImageSource renderImage  = TileTextureImage::Load(*this, db);

    if (renderImage.IsValid())
        m_textureImage = TileTextureImage::Create(std::move(renderImage));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d TileMesh::GetTriangleRange(TileTriangleCR triangle) const
    {
    return DRange3d::From (m_points.at (triangle.m_indices[0]), 
                           m_points.at (triangle.m_indices[1]),
                           m_points.at (triangle.m_indices[2]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3d TileMesh::GetTriangleNormal(TileTriangleCR triangle) const
    {
    return DVec3d::FromNormalizedCrossProductToPoints (m_points.at (triangle.m_indices[0]), 
                                                       m_points.at (triangle.m_indices[1]),
                                                       m_points.at (triangle.m_indices[2]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileMesh::HasNonPlanarNormals() const
    {
    if (m_normals.empty())
        return false;

    for (auto& triangle : m_triangles)
        if (!m_normals.at (triangle.m_indices[0]).IsEqual (m_normals.at (triangle.m_indices[1])) ||
            !m_normals.at (triangle.m_indices[0]).IsEqual (m_normals.at (triangle.m_indices[2])) ||
            !m_normals.at (triangle.m_indices[1]).IsEqual (m_normals.at (triangle.m_indices[2])))
            return true;

    return false;
    }

#if defined(WIP_TILETREE_PUBLISH)
/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Ray.Bentley     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void    TileMesh::AddRenderTile(Render::IGraphicBuilder::TileCorners const& tileCorners, TransformCR transform)
    {
    for (size_t i=0; i<4; i++)
        m_points.push_back(tileCorners.m_pts[i]);

    transform.Multiply (m_points, m_points);                                                                                                                                                                                                                      
    m_uvParams.push_back(DPoint2d::From(0.0, 0.0));
    m_uvParams.push_back(DPoint2d::From(1.0, 0.0));
    m_uvParams.push_back(DPoint2d::From(0.0, 1.0));
    m_uvParams.push_back(DPoint2d::From(1.0, 1.0));

    AddTriangle(TileTriangle(0, 1, 2, false));
    AddTriangle(TileTriangle(1, 3, 2, false));
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Ray.Bentley     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void TileMesh::AddTriMesh(Render::IGraphicBuilder::TriMeshArgs const& triMesh, TransformCR transform, bool invertVParam)
    {
    m_points.resize(triMesh.m_numPoints);

    if (nullptr != triMesh.m_normals)
        m_normals.resize(triMesh.m_numPoints);

    if (nullptr != triMesh.m_textureUV)
        m_uvParams.resize(triMesh.m_numPoints);

    for (int32_t i=0; i<triMesh.m_numPoints; i++)
        {
        transform.Multiply (m_points.at(i), DPoint3d::From((double) triMesh.m_points[i].x, (double) triMesh.m_points[i].y, (double) triMesh.m_points[i].z));
        if (nullptr != triMesh.m_normals)
            m_normals.at(i).Init((double) triMesh.m_normals[i].x, (double) triMesh.m_normals[i].y, (double) triMesh.m_normals[i].z);

        if (nullptr != triMesh.m_textureUV)
            m_uvParams.at(i).Init((double) triMesh.m_textureUV[i].x, (double) (invertVParam ? (1.0 - triMesh.m_textureUV[i].y) : triMesh.m_textureUV[i].y));
        }
    
    for (int32_t i=0; i<triMesh.m_numIndices; i += 3)
        AddTriangle(TileTriangle(triMesh.m_vertIndex[i], triMesh.m_vertIndex[i+1], triMesh.m_vertIndex[i+2], false));
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void    TileMesh::AddMesh (TileMeshCR mesh)
    {
    if (mesh.m_points.empty() ||
        m_normals.empty() != mesh.m_normals.empty() ||
        m_uvParams.empty() != mesh.m_uvParams.empty() ||
        m_attributes.empty() != mesh.m_attributes.empty())
        {
        BeAssert (false && "add mesh empty or not compatible");
        }
    size_t      baseIndex = m_points.size();

    m_points.insert (m_points.end(), mesh.m_points.begin(), mesh.m_points.end());
    if (!mesh.m_normals.empty())
        m_normals.insert (m_normals.end(), mesh.m_normals.begin(), mesh.m_normals.end());

    if (!mesh.m_uvParams.empty())
        m_uvParams.insert (m_uvParams.end(), mesh.m_uvParams.begin(), mesh.m_uvParams.end());

    if (!mesh.m_attributes.empty())
        m_attributes.insert(m_attributes.end(), mesh.m_attributes.begin(), mesh.m_attributes.end());

    for (auto& triangle : mesh.m_triangles)
        AddTriangle (TileTriangle (triangle.m_indices[0] + baseIndex, triangle.m_indices[1] + baseIndex, triangle.m_indices[2] + baseIndex, triangle.m_singleSided));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TileMeshPointCloud::TileMeshPointCloud(TileDisplayParamsCR params, DPoint3dCP points, Rgb const* colors, FPoint3dCP normals, size_t nPoints, TransformCR transform, double clusterTolerance) :  m_displayParams(&params)
    {
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
    PointComparator                     comparator(clusterTolerance);
    bset <DPoint3d, PointComparator>    clusteredPointSet(comparator);

    for (size_t i=0; i<nPoints; i++)
        {
        DPoint3d        testPoint;

        transform.Multiply (testPoint, points[i]);

        if (clusteredPointSet.find(testPoint) == clusteredPointSet.end())
            {
            m_points.push_back(testPoint);
            if (nullptr != colors)
                m_colors.push_back(colors[i]);

            if (nullptr != normals)
                m_normals.push_back(normals[i]);
    
            clusteredPointSet.insert(testPoint);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool    TileMesh::RemoveEntityGeometry (bset<DgnElementId> const& deleteIds)
    {
#if defined(TODO_ATTRIBUTES)
    bool                        deleteGeometryFound = false;
    bmap<uint32_t, uint32_t>    indexRemap;
    bvector<DPoint3d>           savePoints = m_points;
    bvector<DVec3d>             saveNormals = m_normals;
    bvector<DPoint2d>           saveParams = m_uvParams;
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
        

    for (bvector<TileTriangle>::iterator  triangle = m_triangles.begin(); triangle != m_triangles.end(); )
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
    for (bvector<TilePolyline>::iterator  polyline = m_polylines.begin(); polyline != m_polylines.end(); )
        {
        if (indexRemap.find(polyline->m_indices[0]) == indexRemap.end())
            {
            m_polylines.erase (polyline);
            }
        else
            {
            for (size_t i=0; i<polyline->m_indices.size(); i++)
                polyline->m_indices[i] = indexRemap[polyline->m_indices[i]];

            polyline++;
            }                                                                                                         
        }
#endif

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t TileMesh::AddVertex(DPoint3dCR point, DVec3dCP normal, DPoint2dCP param, uint16_t attribute, uint32_t color)
    {
    auto index = static_cast<uint32_t>(m_points.size());

    m_points.push_back(point);
    m_attributes.push_back(attribute);

    if (nullptr != normal)
        m_normals.push_back(*normal);

    if (nullptr != param)
        m_uvParams.push_back(*param);
    else
        m_colors.push_back(m_colorIndex.GetIndex(color));

    m_validIdsPresent |= (0 != attribute);
    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileMeshBuilder::VertexKey::Comparator::operator()(VertexKey const& lhs, VertexKey const& rhs) const
    {
    static const double s_normalTolerance = .1;     
    static const double s_paramTolerance  = .1;

    COMPARE_VALUES(lhs.m_color, rhs.m_color);

    COMPARE_VALUES (lhs.m_attributes.GetElementId(), rhs.m_attributes.GetElementId());
    COMPARE_VALUES (lhs.m_attributes.GetSubCategoryId(), rhs.m_attributes.GetSubCategoryId());
    COMPARE_VALUES (lhs.m_attributes.GetClass(), rhs.m_attributes.GetClass());

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
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileMeshBuilder::TileTriangleKey::TileTriangleKey(TileTriangleCR triangle)
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
bool TileMeshBuilder::TileTriangleKey::operator<(TileTriangleKey const& rhs) const
    {
    COMPARE_VALUES (m_sortedIndices[0], rhs.m_sortedIndices[0]);
    COMPARE_VALUES (m_sortedIndices[1], rhs.m_sortedIndices[1]);
    COMPARE_VALUES (m_sortedIndices[2], rhs.m_sortedIndices[2]);

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileMeshBuilder::AddTriangle(TileTriangleCR triangle)
    {
    if (triangle.IsDegenerate())
        return;

    TileTriangleKey key(triangle);

    if (m_triangleSet.insert(key).second)
        m_mesh->AddTriangle(triangle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileMeshBuilder::GetMaterial(DgnMaterialId materialId, DgnDbR dgnDb)
    {
    if (!materialId.IsValid())
        return false;

    m_materialEl = DgnMaterial::Get(dgnDb, materialId);
    BeAssert(m_materialEl.IsValid());
    m_material = &m_materialEl->GetRenderingAsset();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileMeshBuilder::AddTriangle(PolyfaceVisitorR visitor, DgnMaterialId materialId, DgnDbR dgnDb, FeatureAttributesCR attributes, bool doVertexCluster, bool includeParams, uint32_t fillColor)
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

    TileTriangle            newTriangle(!visitor.GetTwoSided());
    bvector<DPoint2d>       params = visitor.Param();

    if (includeParams &&
        !params.empty() &&
        (m_material || GetMaterial(materialId, dgnDb)))
        {
        auto patternMap = m_material->GetPatternMap();
        bvector<DPoint2d>   computedParams;

        if (patternMap.IsValid())
            {
            BeAssert (m_mesh->Points().empty() || !m_mesh->Params().empty());
            if (SUCCESS == patternMap.ComputeUVParams (computedParams, visitor, &m_transformToDgn))
                params = computedParams;
            }
        }
            
    bool haveNormals = !visitor.Normal().empty();
    for (size_t i = 0; i < 3; i++)
        {
        VertexKey vertex(points.at(i), haveNormals ? &visitor.Normal().at(i) : nullptr, !includeParams || params.empty() ? nullptr : &params.at(i), attributes, fillColor);
        newTriangle.m_indices[i] = doVertexCluster ? AddClusteredVertex(vertex) : AddVertex(vertex);
        }

    BeAssert(m_mesh->Params().empty() || m_mesh->Params().size() == m_mesh->Points().size());
    BeAssert(m_mesh->Normals().empty() || m_mesh->Normals().size() == m_mesh->Points().size());

    AddTriangle(newTriangle);
    ++m_triangleIndex;
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TileMeshBuilder::AddPolyline (bvector<DPoint3d>const& points, FeatureAttributesCR attributes, bool doVertexCluster, uint32_t fillColor)
    {
    TilePolyline    newPolyline;

    for (auto& point : points)
        {
        VertexKey vertex(point, nullptr, nullptr, attributes, fillColor);
        newPolyline.m_indices.push_back (doVertexCluster ? AddClusteredVertex(vertex) : AddVertex(vertex));
        }

    m_mesh->AddPolyline (newPolyline);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TileMeshBuilder::AddPolyface (PolyfaceQueryCR polyface, DgnMaterialId materialId, DgnDbR dgnDb, FeatureAttributesCR attributes, bool includeParams, uint32_t fillColor)
    {
    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(polyface); visitor->AdvanceToNextFace(); )
        AddTriangle(*visitor, materialId, dgnDb, attributes, false, includeParams, fillColor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t TileMeshBuilder::AddVertex(VertexKey const& vertex)
    {
    auto found = m_unclusteredVertexMap.find(vertex);
    if (m_unclusteredVertexMap.end() != found)
        return found->second;

    auto index = m_mesh->AddVertex(vertex.m_point, vertex.GetNormal(), vertex.GetParam(), m_attributes.GetIndex(vertex.m_attributes), vertex.m_color);
    m_unclusteredVertexMap[vertex] = index;
    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t TileMeshBuilder::AddClusteredVertex(VertexKey const& vertex)
    {
    auto found = m_clusteredVertexMap.find(vertex);
    if (m_clusteredVertexMap.end() != found)
        return found->second;

    auto index = m_mesh->AddVertex(vertex.m_point, vertex.GetNormal(), vertex.GetParam(), m_attributes.GetIndex(vertex.m_attributes), vertex.m_color);
    m_clusteredVertexMap[vertex] = index;
    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
WString TileNode::GetNameSuffix() const
    {
    WString suffix;

    if (nullptr != m_parent)
        {
        suffix = WPrintfString(L"%02d", static_cast<int>(m_siblingIndex));
        for (auto parent = m_parent; nullptr != parent->GetParent(); parent = parent->GetParent())
            suffix = WPrintfString(L"%02d", static_cast<int>(parent->GetSiblingIndex())) + suffix;
        }

    return suffix;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
WString TileNode::GetFileName (WCharCP rootName, WCharCP extension) const
    {
    WString     fileName;

    BeFileName::BuildName (fileName, nullptr, nullptr, (rootName + GetNameSuffix()).c_str(), extension);

    return fileName;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IFacetOptionsPtr TileGenerator::CreateTileFacetOptions(double chordTolerance)
    {
    static double       s_defaultAngleTolerance = msGeomConst_piOver2;
    IFacetOptionsPtr    opts = IFacetOptions::Create();

    opts->SetChordTolerance(chordTolerance);
    opts->SetAngleTolerance(s_defaultAngleTolerance);
    opts->SetMaxPerFace(3);
    opts->SetCurvedSurfaceMaxPerFace(3);
    opts->SetParamsRequired(true);
    opts->SetNormalsRequired(true);
    opts->SetBsplineSurfaceEdgeHiding(0);

    return opts;
    }

typedef bmap<TileMeshMergeKey, TileMeshBuilderPtr> MeshBuilderMap;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t TileNode::GetNodeCount() const
    {
    size_t count = 1;
    for (auto const& child : m_children)
        count += child->GetNodeCount();

    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileNodeCP TileNode::GetRoot() const
    {
    auto cur = this;
    auto parent = cur->GetParent();
    while (nullptr != parent)
        {
        cur = parent;
        parent = cur->GetParent();
        }

    return cur;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t TileNode::GetMaxDepth() const
    {
    size_t maxChildDepth = 0;
    for (auto const& child : m_children)
        {
        size_t childDepth = child->GetMaxDepth();
        maxChildDepth = std::max(maxChildDepth, childDepth);
        }

    return 1 + maxChildDepth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileNode::GetTiles(TileNodePList& tiles)
    {
    tiles.push_back(this);
    for (auto& child : m_children)
        child->GetTiles(tiles);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileNodePList TileNode::GetTiles()
    {
    TileNodePList tiles;
    GetTiles(tiles);
    return tiles;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometry::TileGeometry(TransformCR tf, DRange3dCR range, DgnElementId entityId, TileDisplayParamsCR params, bool isCurved, DgnDbR db)
    : m_params(&params), m_transform(tf), m_tileRange(range), m_entityId(entityId), m_isCurved(isCurved), m_facetCount(0), m_hasTexture(params.HasTexture(db))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t TileGeometry::GetFacetCount(IFacetOptionsR options) const
    {
    if (0 != m_facetCount)
        return m_facetCount;
    
    FacetCounter counter(options);
    return (m_facetCount = _GetFacetCount(counter));
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


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometry::TileStrokes::Transform(TransformCR transform)
    {
    for (auto& stroke : m_strokes)
        transform.Multiply (stroke, stroke);
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct PrimitiveTileGeometry : TileGeometry
{
private:
    IGeometryPtr        m_geometry;
    bool                m_curvesAsWire;

    PrimitiveTileGeometry(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsCR params, bool isCurved, bool curvesAsWire, DgnDbR db)
        : TileGeometry(tf, range, elemId, params, isCurved, db), m_geometry(&geometry), m_curvesAsWire(curvesAsWire) { }

    T_TilePolyfaces _GetPolyfaces(IFacetOptionsR facetOptions) override;
    bool _DoDecimate () const override { return m_geometry->GetAsPolyfaceHeader().IsValid(); }
    size_t _GetFacetCount(FacetCounter& counter) const override { return counter.GetFacetCount(*m_geometry); }
    T_TileStrokes _GetStrokes (IFacetOptionsR facetOptions) override;
public:
    static TileGeometryPtr Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsCR params, bool isCurved, bool curvesAsWire, DgnDbR db)
        {
        return new PrimitiveTileGeometry(geometry, tf, range, elemId, params, isCurved, curvesAsWire, db);
        }
};

    
//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct SolidKernelTileGeometry : TileGeometry
{
private:
    IBRepEntityPtr      m_entity;
    BeMutex             m_mutex;

    SolidKernelTileGeometry(IBRepEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsCR params, DgnDbR db)
        : TileGeometry(tf, range, elemId, params, BRepUtil::HasCurvedFaceOrEdge(solid), db), m_entity(&solid) { }

    T_TilePolyfaces _GetPolyfaces(IFacetOptionsR facetOptions) override;
    size_t _GetFacetCount(FacetCounter& counter) const override { return counter.GetFacetCount(*m_entity); }
public:
    static TileGeometryPtr Create(IBRepEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsCR params, DgnDbR db)
        {
        return new SolidKernelTileGeometry(solid, tf, range, elemId, params, db);
        }
};

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     11/2016
//=======================================================================================
struct TextStringTileGeometry : TileGeometry
{
private:
    TextStringPtr                   m_text;
    mutable bvector<CurveVectorPtr> m_glyphCurves;

    TextStringTileGeometry(TextStringR text, TransformCR transform, DRange3dCR range, DgnElementId elemId, TileDisplayParamsCR params, DgnDbR db)
        : TileGeometry(transform, range, elemId, params, true, db), m_text(&text) 
        { 
        InitGlyphCurves();     // Should be able to defer this when font threaded ness is resolved.
        }

    bool _DoVertexCluster() const override { return false; }

public:
    static TileGeometryPtr Create(TextStringR textString, TransformCR transform, DRange3dCR range, DgnElementId elemId, TileDisplayParamsCR params, DgnDbR db)
        {
        return new TextStringTileGeometry(textString, transform, range, elemId, params, db);
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
T_TilePolyfaces _GetPolyfaces(IFacetOptionsR facetOptions) override
    {

    T_TilePolyfaces             polyfaces;
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
        polyfaces.push_back (TileGeometry::TilePolyface (GetDisplayParams(), *polyface));
        }

    return polyfaces;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
T_TileStrokes _GetStrokes (IFacetOptionsR facetOptions) override
    {
    T_TileStrokes               strokes;

    if (DoGlyphBoxes(facetOptions))
        return strokes;

    InitGlyphCurves();

    bvector<bvector<DPoint3d>>  strokePoints;
    Transform                   transform = Transform::FromProduct (GetTransform(), m_text->ComputeTransform());

    for (auto& glyphCurve : m_glyphCurves)
        if (!glyphCurve->IsAnyRegionType())
            collectCurveStrokes(strokePoints, *glyphCurve, facetOptions, transform);

    if (!strokePoints.empty())             
        strokes.push_back (TileGeometry::TileStrokes (GetDisplayParams(), std::move(strokePoints)));

    return strokes;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t _GetFacetCount(FacetCounter& counter) const override 
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
};  // TextStringTileGeometry


//=======================================================================================
// @bsistruct                                                   Ray.Bentley     11/2016
//=======================================================================================
struct GeomPartInstanceTileGeometry : TileGeometry
{
private:
    TileGeomPartPtr     m_part;

    GeomPartInstanceTileGeometry(TileGeomPartR  part, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsCR params, DgnDbR db)
        : TileGeometry(tf, range, elemId, params, part.IsCurved(), db), m_part(&part) { }

public:
    static TileGeometryPtr Create(TileGeomPartR  part, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsCR params, DgnDbR db)  { return new GeomPartInstanceTileGeometry(part, tf, range, elemId, params, db); }

    T_TilePolyfaces _GetPolyfaces(IFacetOptionsR facetOptions) override { return m_part->GetPolyfaces(facetOptions, *this); }
    T_TileStrokes _GetStrokes (IFacetOptionsR facetOptions) override { return m_part->GetStrokes(facetOptions, *this); }
    size_t _GetFacetCount(FacetCounter& counter) const override { return m_part->GetFacetCount (counter) / m_part->GetInstanceCount(); }
    TileGeomPartCPtr _GetPart() const override { return m_part; }

};  // GeomPartInstanceTileGeometry 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
static int     compareDoubleArray (double const* array1, double const* array2, size_t count, double tolerance)
    {
    for (uint32_t i=0; i<count; i++, array1++, array2++)
        if (*array1 < *array2 - tolerance)
            return -1;
        else if (*array1 > *array2 + tolerance)
            return 1;

    return 0;
    }

    static const double     s_compareTolerance = 1.0E-5;

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     12/2016
//=======================================================================================
struct SolidPrimitivePartMapKey 
{
    ISolidPrimitivePtr      m_solidPrimitive;
    DRange3d                m_range;
    TileDisplayParamsCPtr   m_displayParams;


    SolidPrimitivePartMapKey() { }
    SolidPrimitivePartMapKey(ISolidPrimitiveR solidPrimitive, DRange3dCR range, TileDisplayParamsCR displayParams) : m_range(range), m_solidPrimitive(&solidPrimitive), m_displayParams(&displayParams) { }

    bool operator < (SolidPrimitivePartMapKey const& rhs) const { return compareDoubleArray (&m_range.low.x, &rhs.m_range.low.x, 6, s_compareTolerance) < 0; }
    
    bool IsEqual (SolidPrimitivePartMapKey const& other) const { return !(*m_displayParams < *other.m_displayParams) && !(*other.m_displayParams < *m_displayParams) && m_solidPrimitive->IsSameStructureAndGeometry(*other.m_solidPrimitive, s_compareTolerance); }

};



//=======================================================================================
// @bsistruct                                                   Ray.Bentley     12/2016
//=======================================================================================
struct  SolidPrimitivePartMap : bmultimap<SolidPrimitivePartMapKey,  TileGeomPartPtr>
{
    TileGeomPartPtr Find (SolidPrimitivePartMapKey const& key)
        {
        auto const&   range = equal_range (key);

        for (iterator curr = range.first; curr != range.second; ++curr)
            {
            if (curr->first.IsEqual(key))
                return curr->second;
            }

        return TileGeomPartPtr();
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryPtr TileGeometry::Create(TextStringR textString, TransformCR transform, DRange3dCR range, DgnElementId entityId, TileDisplayParamsCR params, DgnDbR db)
    {
    return TextStringTileGeometry::Create(textString, transform, range, entityId, params, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryPtr TileGeometry::Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId entityId, TileDisplayParamsCR params, bool isCurved, bool curvesAsWire, DgnDbR db)
    {
    return PrimitiveTileGeometry::Create(geometry, tf, range, entityId, params, isCurved, curvesAsWire, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryPtr TileGeometry::Create(IBRepEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId entityId, TileDisplayParamsCR params, DgnDbR db)
    {
    return SolidKernelTileGeometry::Create(solid, tf, range, entityId, params, db);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryPtr TileGeometry::Create(TileGeomPartR part, TransformCR transform, DRange3dCR range, DgnElementId entityId, TileDisplayParamsCR params, DgnDbR db)
    {
    return GeomPartInstanceTileGeometry::Create(part, transform, range, entityId, params, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometry::T_TilePolyfaces PrimitiveTileGeometry::_GetPolyfaces(IFacetOptionsR facetOptions)
    {
    PolyfaceHeaderPtr polyface = m_geometry->GetAsPolyfaceHeader();
    
    if (polyface.IsValid())
        {
        if (!HasTexture())
            polyface->ClearParameters(false);

        BeAssertOnce(GetTransform().IsIdentity()); // Polyfaces are transformed during collection.
        return TileGeometry::T_TilePolyfaces (1, TileGeometry::TilePolyface (GetDisplayParams(), *polyface));
        }

    CurveVectorPtr      curveVector = m_geometry->GetAsCurveVector();

    if (curveVector.IsValid() && !curveVector->IsAnyRegionType())       // Non region curveVectors....
        return T_TilePolyfaces();

    IPolyfaceConstructionPtr polyfaceBuilder = IPolyfaceConstruction::Create(facetOptions);

    ISolidPrimitivePtr  solidPrimitive = curveVector.IsNull() ? m_geometry->GetAsISolidPrimitive() : nullptr;
    MSBsplineSurfacePtr bsplineSurface = solidPrimitive.IsNull() && curveVector.IsNull() ? m_geometry->GetAsMSBsplineSurface() : nullptr;

    if (curveVector.IsValid() && !m_curvesAsWire)
        polyfaceBuilder->AddRegion(*curveVector);
    else if (solidPrimitive.IsValid())
        polyfaceBuilder->AddSolidPrimitive(*solidPrimitive);
    else if (bsplineSurface.IsValid())
        polyfaceBuilder->Add(*bsplineSurface);

    T_TilePolyfaces     polyfaces;

    polyface = polyfaceBuilder->GetClientMeshPtr();
    if (polyface.IsValid())
        {
        polyface->Transform(GetTransform());
        polyfaces.push_back (TileGeometry::TilePolyface (GetDisplayParams(), *polyface));
        }
    return polyfaces;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometry::T_TileStrokes PrimitiveTileGeometry::_GetStrokes (IFacetOptionsR facetOptions)
    {
    CurveVectorPtr      curveVector = m_geometry->GetAsCurveVector();
    T_TileStrokes       tileStrokes;

    if (curveVector.IsValid() && (m_curvesAsWire || !curveVector->IsAnyRegionType()))
        {
        bvector<bvector<DPoint3d>>  strokePoints;

        collectCurveStrokes(strokePoints, *curveVector, facetOptions, GetTransform());

        if (!strokePoints.empty())
            tileStrokes.push_back (TileGeometry::TileStrokes (GetDisplayParams(), std::move(strokePoints)));
        }

    return tileStrokes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometry::T_TilePolyfaces SolidKernelTileGeometry::_GetPolyfaces(IFacetOptionsR facetOptions)
    {
#if defined (BENTLEYCONFIG_PARASOLID)    
    // Cannot process the same solid entity simultaneously from multiple threads...
    BeMutexHolder lock(m_mutex);

    DRange3d entityRange = m_entity->GetEntityRange();
    if (entityRange.IsNull())
        return TileGeometry::T_TilePolyfaces();;

    double              rangeDiagonal = entityRange.DiagonalDistance();
    static double       s_minRangeRelTol = 1.0e-4;
    double              minChordTolerance = rangeDiagonal * s_minRangeRelTol;
    IFacetOptionsPtr    pFacetOptions = facetOptions.Clone();
    
    if (facetOptions.GetChordTolerance() < minChordTolerance)
        pFacetOptions->SetChordTolerance (minChordTolerance);

    pFacetOptions->SetParamsRequired (true); // Can't rely on HasTexture due to face attached material that may have texture.

    TileGeometry::T_TilePolyfaces   tilePolyfaces;

    if (nullptr != m_entity->GetFaceMaterialAttachments())
        {
        bvector<PolyfaceHeaderPtr>  polyfaces;
        bvector<FaceAttachment>     params;

        if (!BRepUtil::FacetEntity(*m_entity, polyfaces, params, *pFacetOptions))
            return TileGeometry::T_TilePolyfaces();;

        GeometryParams baseParams;

        // Require valid category/subcategory for sub-category appearance color/material...
        baseParams.SetCategoryId(GetDisplayParams().GetCategoryId());
        baseParams.SetSubCategoryId(GetDisplayParams().GetSubCategoryId());
        baseParams.SetGeometryClass(GetDisplayParams().GetClass());

        for (size_t i=0; i<polyfaces.size(); i++)
            {
            auto&   polyface = polyfaces[i];

            if (polyface->HasFacets())
                {
                GeometryParams faceParams;
                params[i].ToGeometryParams(faceParams, baseParams);

                TileDisplayParamsCPtr displayParams = TileDisplayParams::Create(GetDisplayParams().GetColor(), faceParams);
                tilePolyfaces.push_back (TileGeometry::TilePolyface (*displayParams, *polyface));
                }
            }
        }
    else
        {
        auto polyface = BRepUtil::FacetEntity(*m_entity, *pFacetOptions);
    
        if (polyface.IsValid() && polyface->HasFacets())
            tilePolyfaces.push_back (TileGeometry::TilePolyface (GetDisplayParams(), *polyface));

        }

    if (!GetTransform().IsIdentity())
        for (auto& tilePolyface : tilePolyfaces)
            tilePolyface.m_polyface->Transform (GetTransform());

    return tilePolyfaces;

#else
    return TileGeometry::T_TilePolyfaces();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometry::T_TilePolyfaces TileGeometry::GetPolyfaces(double chordTolerance, NormalMode normalMode)
    {
    return _GetPolyfaces (*CreateFacetOptions(chordTolerance, normalMode));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
IFacetOptionsPtr TileGeometry::CreateFacetOptions(double chordTolerance, NormalMode normalMode) const
    {
    auto facetOptions = TileGenerator::CreateTileFacetOptions(chordTolerance / m_transform.ColumnXMagnitude());
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
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::TileGenerator(DgnDbR dgndb, ITileGenerationFilterP filter, ITileGenerationProgressMonitorP progress)
    : m_progressMeter(nullptr != progress ? *progress : s_defaultProgressMeter), m_dgndb(dgndb), 
      m_totalTiles(0), m_totalModels(0), m_completedModels(0)
    {
    DPoint3d origin = dgndb.GeoLocation().GetProjectExtents().GetCenter();
    m_spatialTransformFromDgn = Transform::From(-origin.x, -origin.y, -origin.z);
    }

#if defined (BENTLEYCONFIG_PARASOLID) 
/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     08/2009
+===============+===============+===============+===============+===============+========*/
struct PSolidPartitionMark
{
    PK_PARTITION_t                      m_originalPartition;
    PK_PARTITION_t                      m_lightweightPartition;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
PSolidPartitionMark ()
    {
    PK_SESSION_ask_curr_partition (&m_originalPartition);

    if (SUCCESS == PK_PARTITION_create_empty(&m_lightweightPartition))
        {
        PK_PARTITION_set_type(m_lightweightPartition, PK_PARTITION_type_light_c);
        PK_PARTITION_set_current(m_lightweightPartition);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
~PSolidPartitionMark ()
    {
    PK_PARTITION_delete_o_t options;

    PK_PARTITION_delete_o_m (options);

    options.delete_non_empty = true;
    /* unused - StatusInt   status = */PK_PARTITION_delete (m_lightweightPartition, &options);

    PK_PARTITION_set_current (m_originalPartition);
    }
};
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeneratorStatus TileGenerator::GenerateTiles(ITileCollector& collector, DgnModelIdSet const& modelIds, double leafTolerance, bool surfacesOnly, size_t maxPointsPerTile)
    {
    auto nModels = static_cast<uint32_t>(modelIds.size());
    if (0 == nModels)
        return TileGeneratorStatus::NoGeometry;

    // unused - auto nCompletedModels = 0;

    T_HOST.GetFontAdmin().EnsureInitialized();
    GetDgnDb().Fonts().Update();

#if defined (BENTLEYCONFIG_PARASOLID) 
    PSolidPartitionMark     partitionMark;
    ThreadedLocalParasolidHandlerStorageMark  parasolidParasolidHandlerStorageMark;
    PSolidKernelManager::StartSession();
#endif

    StopWatch timer(true);

    m_totalModels = nModels;
    m_progressMeter._IndicateProgress(0, nModels);

    auto future = GenerateTilesFromModels(collector, modelIds, leafTolerance, surfacesOnly, maxPointsPerTile);
    /* unused - auto status = */ future.get();

    m_completedModels.store(0);
    m_totalModels = 0;

    m_statistics.m_tileGenerationTime = timer.GetCurrentSeconds();
    m_statistics.m_tileCount = m_totalTiles;
    m_totalTiles.store(0);

    m_progressMeter._IndicateProgress(nModels, nModels);

    return TileGeneratorStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::FutureStatus TileGenerator::GenerateTilesFromModels(ITileCollector& collector, DgnModelIdSet const& modelIds, double leafTolerance, bool surfacesOnly, size_t maxPointsPerTile)
    {
    std::vector<FutureStatus> modelFutures;
    for (auto const& modelId : modelIds)
        {
        auto model = GetDgnDb().Models().GetModel(modelId);
        if (model.IsValid())
            modelFutures.push_back(GenerateTiles(collector, leafTolerance, surfacesOnly, maxPointsPerTile, *model));
        }

    return folly::unorderedReduce(modelFutures, TileGeneratorStatus::Success, [=](TileGeneratorStatus reduced, TileGeneratorStatus next)
        {
        return TileGeneratorStatus::Aborted == reduced || TileGeneratorStatus::Aborted == next ? TileGeneratorStatus::Aborted : TileGeneratorStatus::Success;
        });
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::FutureStatus TileGenerator::GenerateTiles(ITileCollector& collector, double leafTolerance, bool surfacesOnly, size_t maxPointsPerTile, DgnModelR model)
    {
    DgnModelPtr         modelPtr(&model);
    auto                pCollector = &collector;
    auto                generateMeshTiles = dynamic_cast<IGenerateMeshTiles*>(&model);
    GeometricModelP     geometricModel = model.ToGeometricModelP();
    bool                isModel3d = nullptr != geometricModel->ToGeometricModel3d();
    
    if (!isModel3d)
        surfacesOnly = false;


    if (nullptr == geometricModel)
        {
        BeAssert (false);
        return folly::makeFuture(TileGeneratorStatus::NoGeometry);
        }

    double          rangeDiagonal = geometricModel->QueryModelRange().DiagonalDistance();
    double          minDiagonalToleranceRatio = isModel3d ? 1.0E-3 : 1.0E-5;   // Don't allow leaf tolerance to be less than this factor times range diagonal.
    static  double  s_minLeafTolerance = 1.0E-6;

    leafTolerance = std::max(s_minLeafTolerance, std::min(leafTolerance, rangeDiagonal * minDiagonalToleranceRatio));

#ifdef WIP_RBB
    return GenerateTilesFromTileTree (&collector, leafTolerance, surfacesOnly, geometricModel);
#else
    return folly::makeFuture(TileGeneratorStatus::NoGeometry);
#endif
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::FutureGenerateTileResult TileGenerator::GenerateElementTiles(ITileCollector& collector, double leafTolerance, bool surfacesOnly, size_t maxPointsPerTile, DgnModelR model)
    {
    auto                cache = TileGenerationCache::Create(TileGenerationCache::Options::CacheGeometrySources);
    ElementTileContext  context(*cache, model, collector, leafTolerance, model.Is3dModel() && surfacesOnly, maxPointsPerTile);

    return PopulateCache(context).then([=](TileGeneratorStatus status)
        {
        return GenerateTileset(status, context);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::FutureStatus TileGenerator::PopulateCache(ElementTileContext context)
    {
    return folly::via(&BeFolly::ThreadPool::GetIoPool(), [=]                                                         
        {
        return context.m_cache->Populate(GetDgnDb(), *context.m_model);
        });
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Transform   TileGenerator::GetTransformFromDgn(DgnModelCR model) const
    {
    return model.IsSpatialModel() ?  GetSpatialTransformFromDgn() : Transform::FromIdentity(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::FutureGenerateTileResult TileGenerator::GenerateTileset(TileGeneratorStatus status, ElementTileContext context)
    {
    auto& cache = *context.m_cache;
    auto sheet = context.m_model->ToSheetModel();

    DRange3d        range = cache.GetRange();
    if (nullptr != sheet)
        range.Extend(sheet->GetSheetExtents());

    if (TileGeneratorStatus::Success != status && nullptr == sheet)
        {
        GenerateTileResult result(status, ElementTileNode::Create(*context.m_model, range, GetTransformFromDgn(*context.m_model), 0, 0, nullptr).get());
        return folly::makeFuture(result);
        }
    
    ElementTileNodePtr parent = ElementTileNode::Create(*context.m_model, range, GetTransformFromDgn(*context.m_model), 0, 0, nullptr);
    return ProcessParentTile(parent, context).then([=](GenerateTileResult result) { return ProcessChildTiles(result.m_status, parent, context); });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::FutureGenerateTileResult TileGenerator::ProcessParentTile
(ElementTileNodePtr parent, ElementTileContext context)
    {
    return folly::via(&BeFolly::ThreadPool::GetIoPool(), [=]()
        {
    #if defined (BENTLEYCONFIG_PARASOLID) 
        ThreadedParasolidErrorHandlerOuterMarkPtr  outerMark = ThreadedParasolidErrorHandlerOuterMark::Create();
        ThreadedParasolidErrorHandlerInnerMarkPtr  innerMark = ThreadedParasolidErrorHandlerInnerMark::Create(); 
    #endif

        auto& tile = *parent;
        auto& collector = *context.m_collector;
        auto leafTolerance = context.m_leafTolerance;
        auto maxPointsPerTile = context.m_maxPointsPerTile;
        auto const& generationCache = *context.m_cache;

        double          tileTolerance = tile.GetDgnRange().DiagonalDistance() / s_minToleranceRatio;
        bool            isLeaf = tileTolerance < leafTolerance && parent->GetChildren().empty();
        bool            leafThresholdExceeded = false;

        // Always collect geometry at the target leaf tolerance.
        // If maxPointsPerTile is exceeded, we will keep that geometry, but adjust this tile's target tolerance
        // Later that tolerance will be used in _GenerateMeshes() to facet appropriately (and to filter out 
        // elements too small to be included in this tile)
        tile.CollectGeometry(generationCache, m_dgndb, &leafThresholdExceeded, leafTolerance, context.m_surfacesOnly, isLeaf ? 0 : maxPointsPerTile); // ###TODO: Check return status

        if (!isLeaf && !leafThresholdExceeded)
            isLeaf = true;

        GenerateTileResult result(m_progressMeter._WasAborted() ? TileGeneratorStatus::Aborted : TileGeneratorStatus::Success, tile.GetRoot());
        if (tile.GetGeometries().empty())
            return result;

        tile.SetIsEmpty(false);
        tile.SetIsLeaf(isLeaf);

        if (isLeaf)
            {
            tile.SetTolerance(leafTolerance);
            collector._AcceptTile(tile);
            tile.ClearGeometry();

            return result;
            }

        bvector<DRange3d>   subRanges;

        tile.ComputeChildTileRanges(subRanges, tile.GetDgnRange());
        for (auto& subRange : subRanges)
            {
            ElementTileNodePtr child = ElementTileNode::Create(tile.GetModel(), subRange, tile.GetTransformFromDgn(), tile.GetDepth()+1, tile.GetChildren().size(), &tile);

            tile.GetChildren().push_back(child);
            }

        tile.AdjustTolerance(tileTolerance);

        collector._AcceptTile(tile);
        tile.ClearGeometry();

        result.m_status = m_progressMeter._WasAborted() ? TileGeneratorStatus::Aborted : TileGeneratorStatus::Success;
        return result;
        });
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementTileNode::AdjustTolerance(double newTolerance)
    {
    if (newTolerance <= GetTolerance())
        return;

    // Change the tolerance at which _GetPolyface() will facet
    SetTolerance(newTolerance);

    // Remove any geometries too small for inclusion in this tile
    double minRangeDiagonal = s_minRangeBoxSize * newTolerance;
    auto eraseAt = std::remove_if(m_geometries.begin(), m_geometries.end(), [=](TileGeometryPtr const& geom) { return geom->GetTileRange().DiagonalDistance() < minRangeDiagonal; });
    if (eraseAt != m_geometries.end())
        m_geometries.erase(eraseAt, m_geometries.end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::FutureGenerateTileResult TileGenerator::ProcessChildTiles(TileGeneratorStatus status, ElementTileNodePtr parent, ElementTileContext context)
    {
#if defined (BENTLEYCONFIG_PARASOLID) 
    ThreadedParasolidErrorHandlerOuterMarkPtr  outerMark = ThreadedParasolidErrorHandlerOuterMark::Create();
    ThreadedParasolidErrorHandlerInnerMarkPtr  innerMark = ThreadedParasolidErrorHandlerInnerMark::Create(); 
#endif

    auto root = static_cast<ElementTileNodeP>(parent->GetRoot());
    if (parent->GetChildren().empty() || TileGeneratorStatus::Success != status)
        return folly::makeFuture(GenerateTileResult(status, root));

    std::vector<FutureGenerateTileResult> childFutures;
    for (auto& child : parent->GetChildren())
        {
        auto elemChild = static_cast<ElementTileNodeP>(child.get());
        auto childFuture = ProcessParentTile(elemChild, context).then([=](GenerateTileResult result) { return ProcessChildTiles(result.m_status, elemChild, context); });
        childFutures.push_back(std::move(childFuture));
        }

    auto result = GenerateTileResult(status, root);
    return folly::unorderedReduce(childFutures, result, [=](GenerateTileResult, GenerateTileResult)
        {
        return GenerateTileResult(m_progressMeter._WasAborted() ? TileGeneratorStatus::Aborted : TileGeneratorStatus::Success, root);
        });
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileNode::ComputeChildTileRanges(bvector<DRange3d>& subRanges, DRange3dCR range, size_t splitCount)
    {
    bvector<DRange3d> bisectRanges;
    DVec3d diagonal = range.DiagonalVector();

    if (diagonal.x > diagonal.y && diagonal.x > diagonal.z)
        {
        double bisectValue = (range.low.x + range.high.x) / 2.0;

        bisectRanges.push_back (DRange3d::From (range.low.x, range.low.y, range.low.z, bisectValue, range.high.y, range.high.z));
        bisectRanges.push_back (DRange3d::From (bisectValue, range.low.y, range.low.z, range.high.x, range.high.y, range.high.z));
        }
    else if (diagonal.y > diagonal.z)
        {
        double bisectValue = (range.low.y + range.high.y) / 2.0;

        bisectRanges.push_back (DRange3d::From (range.low.x, range.low.y, range.low.z, range.high.x, bisectValue, range.high.z));
        bisectRanges.push_back (DRange3d::From (range.low.x, bisectValue, range.low.z, range.high.x, range.high.y, range.high.z));
        }
    else
        {
        double bisectValue = (range.low.z + range.high.z) / 2.0;

        bisectRanges.push_back (DRange3d::From (range.low.x, range.low.y, range.low.z, range.high.x, range.high.y, bisectValue));
        bisectRanges.push_back (DRange3d::From (range.low.x, range.low.y, bisectValue, range.high.x, range.high.y, range.high.z));
        }

    splitCount--;
    for (auto& bisectRange : bisectRanges)
        {
        if (0 == splitCount)
            subRanges.push_back (bisectRange);
        else
            ComputeChildTileRanges(subRanges, bisectRange, splitCount);
        }
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
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct TileGeometryProcessor : IGeometryProcessor
{
private:
    typedef bmap<DgnGeometryPartId, TileGeomPartPtr>  GeomPartMap;

    IFacetOptionsR              m_facetOptions;
    IFacetOptionsPtr            m_targetFacetOptions;
    DgnElementId                m_curElemId;
    TileGenerationCacheCR       m_cache;
    TileDisplayParamsCache      m_displayParamsCache;
    DgnDbR                      m_dgndb;
    TileGeometryList&           m_geometries;
    DRange3d                    m_range;
    DRange3d                    m_tileRange;
    Transform                   m_transformFromDgn;
    TileGeometryList            m_curElemGeometries;
    double                      m_minRangeDiagonal;
    double                      m_minTextBoxSize;
    double                      m_minLineStyleWidth;
    bool*                       m_leafThresholdExceeded;
    size_t                      m_leafCountThreshold;
    size_t                      m_leafCount;
    bool                        m_is2d;
    bool                        m_surfacesOnly;
    GeomPartMap                 m_geomParts;
    SolidPrimitivePartMap       m_solidPrimitiveParts;
    TileDisplayParamsCPtr       m_polyfaceCacheDisplay;
    IPolyfaceConstructionPtr    m_polyfaceCache;

    void PushGeometry(TileGeometryR geom);
    void AddElementGeometry(TileGeometryR geom);
    bool ProcessGeometry(IGeometryR geometry, bool isCurved, bool curvesAsWire, SimplifyGraphic& gf);

    IFacetOptionsP _GetFacetOptionsP() override { return &m_facetOptions; }

    bool _ProcessCurveVector(CurveVectorCR curves, bool filled, SimplifyGraphic& gf) override;
    bool _ProcessSolidPrimitive(ISolidPrimitiveCR prim, SimplifyGraphic& gf) override;
    bool _ProcessSurface(MSBsplineSurfaceCR surface, SimplifyGraphic& gf) override;
    bool _ProcessPolyface(PolyfaceQueryCR polyface, bool filled, SimplifyGraphic& gf) override;
    bool _ProcessBody(IBRepEntityCR solid, SimplifyGraphic& gf) override;
    bool _ProcessTextString(TextStringCR, SimplifyGraphic&) override;

    double _AdjustZDepth(double zDepthIn) override
        {
        // zDepth is obtained from GeometryParams::GetNetDisplayPriority(), which returns an int32_t.
        // Coming from mstn, priorities tend to be in [-500..500]
        // Let's assume that mstn's range is the full range and clamp anything outside that.
        // Map them to [-s_half2dDepthRange, s_half2dDepthRange]
        constexpr double priorityRange = 500;
        constexpr double ratio = s_half2dDepthRange / priorityRange;

        auto zDepth = std::min(zDepthIn, priorityRange);
        zDepth = std::max(zDepth, -priorityRange);

        return zDepth * ratio;
        }

    UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR, SimplifyGraphic&) const override { return UnhandledPreference::Facet; }
    UnhandledPreference _GetUnhandledPreference(CurveVectorCR, SimplifyGraphic&)     const override { return UnhandledPreference::Facet; }
    UnhandledPreference _GetUnhandledPreference(IBRepEntityCR, SimplifyGraphic&)     const override { return UnhandledPreference::Facet; }

public:
    TileGeometryProcessor(TileGeometryList& geometries, TileGenerationCacheCR cache, DgnDbR db, DRange3dCR range, IFacetOptionsR facetOptions, TransformCR transformFromDgn, bool* leafThresholdExceeded, double tolerance, bool surfacesOnly, size_t leafCountThreshold, bool is2d) 
        : m_geometries (geometries), m_facetOptions(facetOptions), m_targetFacetOptions(facetOptions.Clone()), m_cache(cache), m_dgndb(db), m_range(range), m_transformFromDgn(transformFromDgn),
          m_leafThresholdExceeded(leafThresholdExceeded), m_leafCountThreshold(leafCountThreshold), m_leafCount(0), m_is2d(is2d), m_surfacesOnly (surfacesOnly)
        {
        static const double s_minTextBoxToleranceRatio = 1.0;           // Below this ratio to tolerance text is rendered as box.
        static const double s_minLineStyleWidthToleranceRatio = 1.0;     // Below this ratio to tolerance line styles are rendered as continuous.

        m_targetFacetOptions->SetChordTolerance(facetOptions.GetChordTolerance() * transformFromDgn.ColumnXMagnitude());
        m_minRangeDiagonal = s_minRangeBoxSize * tolerance;
        m_minTextBoxSize  = s_minTextBoxToleranceRatio * tolerance;
        m_minLineStyleWidth = s_minLineStyleWidthToleranceRatio * tolerance;
        m_transformFromDgn.Multiply (m_tileRange, m_range);
        }

    void ProcessElement(ViewContextR context, DgnElementId elementId, DRange3dCR range);
    TileGeneratorStatus OutputGraphics(ViewContextR context);

    void AddGeomPart (Render::GraphicBuilderR graphic, DgnGeometryPartCR geomPart, TransformCR subToGraphic, GeometryParamsR geomParams, GraphicParamsR graphicParams, ViewContextR viewContext);
    bool IsGeomPartContained (Render::GraphicBuilderR graphic, DgnGeometryPartCR geomPart, TransformCR subToGraphic) const;
    void FlushPolyfaceCache();
    virtual bool _DoLineStyleStroke(Render::LineStyleSymbCR lineStyleSymb, IFacetOptionsPtr&, SimplifyGraphic&) const override {double maxWidth = lineStyleSymb.GetStyleWidth(); return (0.0 == maxWidth || maxWidth > m_minLineStyleWidth);}

    DgnDbR GetDgnDb() const { return m_dgndb; }
    TileGenerationCacheCR GetCache() const { return m_cache; }

    bool BelowMinRange(DRange3dCR range) const
        {
        // Avoid processing any elements with range smaller than roughly half a pixel...
        return range.DiagonalDistance() < m_minRangeDiagonal;
        }
    
    void PushCurrentGeometry()
        {
        for (auto& geom : m_curElemGeometries)
            PushGeometry(*geom);
        }


    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Ray.Bentley     11/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    UnhandledPreference _GetUnhandledPreference(TextStringCR textString, SimplifyGraphic& simplifyGraphic) const override 
        {
        DRange2d        range = textString.GetRange();
        Transform       transformToTile = Transform::FromProduct(m_transformFromDgn, simplifyGraphic.GetLocalToWorldTransform(), textString.ComputeTransform());
        double          minTileDimension = transformToTile.ColumnXMagnitude() * std::min(range.XLength(), range.YLength());

        return minTileDimension < m_minTextBoxSize ? UnhandledPreference::Box : UnhandledPreference::Curve;
        }

};


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometryProcessor::AddElementGeometry(TileGeometryR geom)
    {
    // ###TODO: Only if geometry caching enabled...
    m_curElemGeometries.push_back(&geom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometryProcessor::PushGeometry(TileGeometryR geom)
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
* @bsimethod                                                    Ray.Bentley     02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::IsGeomPartContained (Render::GraphicBuilderR graphic, DgnGeometryPartCR geomPart, TransformCR subToGraphic) const
    {
    Transform               partToTile = Transform::FromProduct(m_transformFromDgn, graphic.GetLocalToWorldTransform(), subToGraphic);
    DRange3d                partTileRange;

    partToTile.Multiply (partTileRange, geomPart.GetBoundingBox());

    return partTileRange.IsContained (m_tileRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometryProcessor::AddGeomPart (Render::GraphicBuilderR graphic, DgnGeometryPartCR geomPart, TransformCR subToGraphic, GeometryParamsR geomParams, GraphicParamsR graphicParams, ViewContextR viewContext)
    {
    TileGeomPartPtr         tileGeomPart;
    Transform               partToWorld = Transform::FromProduct(graphic.GetLocalToWorldTransform(), subToGraphic);
    TileDisplayParamsCR     displayParams = m_displayParamsCache.Get(graphicParams, geomParams);
    DRange3d                range;
    auto const&             foundPart = m_geomParts.find (geomPart.GetId());

    if (foundPart == m_geomParts.end())
        {
        FlushPolyfaceCache();

        Transform                       inverseLocalToWorld;
        AutoRestore<Transform>          saveTransform (&m_transformFromDgn, Transform::FromIdentity());
        GeometryStreamIO::Collection    collection(geomPart.GetGeometryStream().GetData(), geomPart.GetGeometryStream().GetSize());
        
        inverseLocalToWorld.InverseOf (graphic.GetLocalToWorldTransform());

        auto                            partBuilder = graphic.CreateSubGraphic(inverseLocalToWorld);
        TileGeometryList                saveCurrGeometries = m_curElemGeometries;;
        
        m_curElemGeometries.clear();
        collection.Draw(*partBuilder, viewContext, geomParams, false, &geomPart);
        FlushPolyfaceCache();

        m_geomParts.Insert (geomPart.GetId(), tileGeomPart = TileGeomPart::Create(geomPart.GetBoundingBox(), m_curElemGeometries));
        m_curElemGeometries = saveCurrGeometries;
        }
    else
        {
        tileGeomPart = foundPart->second;
        }

    tileGeomPart->IncrementInstanceCount();

    Transform   tf = Transform::FromProduct(m_transformFromDgn, partToWorld);
    
    tf.Multiply(range, tileGeomPart->GetRange());
    AddElementGeometry(*TileGeometry::Create(*tileGeomPart, tf, range, m_curElemId, displayParams, m_dgndb));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeomPart::TileGeomPart(DRange3dCR range, TileGeometryList const& geometries) :  m_range (range), m_instanceCount(0), m_facetCount(0), m_geometries(geometries)
    { 
    }                

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeomPart::IsWorthInstancing (double chordTolerance) const
    {
    static size_t               s_minInstanceCount = 2;
    static size_t               s_minFacetCompression = 50000;

    if (GetInstanceCount() < s_minInstanceCount)
        return false;

    auto            facetOptions = TileGenerator::CreateTileFacetOptions(chordTolerance);
    FacetCounter    counter(*facetOptions);
    size_t          facetCount = GetFacetCount(counter);

    return (m_instanceCount - 1) * facetCount > s_minFacetCompression;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeomPart::IsCurved() const
    {
    for (auto& geometry : m_geometries)
        if (geometry->IsCurved())
            return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometry::T_TilePolyfaces TileGeomPart::GetPolyfaces(IFacetOptionsR facetOptions, TileGeometryCR instance)
    {
    TileGeometry::T_TilePolyfaces polyfaces;
    
    for (auto& geometry : m_geometries) 
        {
        TileGeometry::T_TilePolyfaces thisPolyfaces = geometry->GetPolyfaces (facetOptions);

        for (auto& thisPolyface : thisPolyfaces)
            {
            auto    polyface = thisPolyface.Clone();

            polyface.Transform(instance.GetTransform());
            polyfaces.push_back (polyface);
            }
        }


    return polyfaces;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometry::T_TileStrokes TileGeomPart::GetStrokes (IFacetOptionsR facetOptions, TileGeometryCR instance)
    {
    TileGeometry::T_TileStrokes strokes;

    for (auto& geometry : m_geometries) 
        {
        TileGeometry::T_TileStrokes   thisStrokes = geometry->GetStrokes(facetOptions);

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
size_t TileGeomPart::GetFacetCount(FacetCounter& counter) const
    {
    if (0 == m_facetCount)
        for (auto& geometry : m_geometries) 
            m_facetCount += geometry->GetFacetCount(counter);
            
    return m_facetCount;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometryProcessor::ProcessElement(ViewContextR context, DgnElementId elemId, DRange3dCR dgnRange)
    {
    try
        {
        m_curElemGeometries.clear();
        bool haveCached = m_cache.GetCachedGeometry(m_curElemGeometries, elemId);
        if (!haveCached)
            {
            m_curElemId = elemId;
            context.VisitElement(elemId, false);
            FlushPolyfaceCache();
            }
        PushCurrentGeometry();
        if (!haveCached)
            m_cache.AddCachedGeometry(elemId, std::move(m_curElemGeometries));
        }
    catch (...)
        {
        // This shouldn't be necessary - but an uncaught interception will cause the processing to continue forever. (OpenCascade error in LargeHatchPlant.)
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::ProcessGeometry(IGeometryR geom, bool isCurved, bool curvesAsWire, SimplifyGraphic& gf)
    {
    DRange3d range;
    if (!geom.TryGetRange(range))
        return false;   // ignore and continue

    auto tf = Transform::FromProduct(m_transformFromDgn, gf.GetLocalToWorldTransform());
    tf.Multiply(range, range);
    
    TileDisplayParamsCR displayParams = m_displayParamsCache.Get(gf.GetCurrentGraphicParams(), gf.GetCurrentGeometryParams(), m_is2d /* Ignore lighting */, curvesAsWire && geom.GetAsCurveVector().IsValid());

    AddElementGeometry(*TileGeometry::Create(geom, tf, range, m_curElemId, displayParams, isCurved, curvesAsWire, m_dgndb));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessCurveVector(CurveVectorCR curves, bool filled, SimplifyGraphic& gf)
    {
    bool        isRegion = curves.IsAnyRegionType();
    bool        isCurved = curves.ContainsNonLinearPrimitive();

    if (m_surfacesOnly && !isRegion)
        return true;

    if (m_is2d)
        {
        CurveVectorPtr clone = curves.Clone();

        // Always treat 2D models as with MicroStation wireframe mode.
        if (filled && isRegion)
            {
            CurveVectorPtr  fillRegion = clone;
            static double   s_blankingRegionOffset = s_half2dDepthRange / 2000.0;  // Arbitrary - but below the a single priority (-500,500).

            if (gf.GetCurrentGraphicParams().IsBlankingRegion())
                fillRegion = clone->Clone(Transform::From(0.0, 0.0, -s_blankingRegionOffset));

            ProcessGeometry(*IGeometry::Create(fillRegion), isCurved, false, gf);
            if (gf.GetCurrentGraphicParams().GetLineColor() == gf.GetCurrentGraphicParams().GetFillColor())
                return true;
            }
        return ProcessGeometry(*IGeometry::Create(clone), isCurved, true, gf);
        }
    else
        {
        if (curves.IsAnyRegionType() && !isCurved && !m_is2d)
            return false;   // process as facets (optimization).

        CurveVectorPtr clone = curves.Clone();
        return ProcessGeometry(*IGeometry::Create(clone), true, !isRegion, gf);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessSolidPrimitive(ISolidPrimitiveCR prim, SimplifyGraphic& gf) 
    {
    bool hasCurvedFaceOrEdge = prim.HasCurvedFaceOrEdge();

    DRange3d                range, thisTileRange;
    ISolidPrimitivePtr      clone = prim.Clone();
    Transform               tf = Transform::FromProduct(m_transformFromDgn, gf.GetLocalToWorldTransform());
    TileDisplayParamsCR     displayParams = m_displayParamsCache.Get(gf.GetCurrentGraphicParams(), gf.GetCurrentGeometryParams(), m_is2d);

    clone->GetRange(range);
    tf.Multiply(thisTileRange, range);

    if (!thisTileRange.IsContained(m_tileRange))
        {
        IGeometryPtr geom = IGeometry::Create(clone);
        return ProcessGeometry(*geom, hasCurvedFaceOrEdge, false, gf);
        }

    
    SolidPrimitivePartMapKey    key(*clone, range, displayParams);
    TileGeomPartPtr             tileGeomPart = m_solidPrimitiveParts.Find(key);

    if (!tileGeomPart.IsValid())
        {
        IGeometryPtr        geom = IGeometry::Create(clone);
        TileGeometryList    geometryList;

        geometryList.push_back(TileGeometry::Create(*geom, Transform::FromIdentity(), range, m_curElemId, displayParams, hasCurvedFaceOrEdge, false, m_dgndb));
        
        m_solidPrimitiveParts.Insert(key, tileGeomPart = TileGeomPart::Create(range, geometryList));
        }
    
    tileGeomPart->IncrementInstanceCount();
    AddElementGeometry(*TileGeometry::Create(*tileGeomPart, tf, thisTileRange, m_curElemId, displayParams, m_dgndb));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessSurface(MSBsplineSurfaceCR surface, SimplifyGraphic& gf) 
    {
    MSBsplineSurfacePtr clone = MSBsplineSurface::CreatePtr();
    clone->CopyFrom(surface);
    IGeometryPtr geom = IGeometry::Create(clone);

    bool isCurved = (clone->GetUOrder() > 2 || clone->GetVOrder() > 2);
    return ProcessGeometry(*geom, isCurved, false, gf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessPolyface(PolyfaceQueryCR polyface, bool filled, SimplifyGraphic& gf) 
    {
    PolyfaceHeaderPtr clone = polyface.Clone();
    if (!clone->IsTriangulated())
        clone->Triangulate();

    clone->Transform(Transform::FromProduct(m_transformFromDgn, gf.GetLocalToWorldTransform()));

    TileDisplayParamsCR displayParams = m_displayParamsCache.Get(gf.GetCurrentGraphicParams(), gf.GetCurrentGeometryParams(), m_is2d);

    if (m_polyfaceCache.IsNull() || !displayParams.IsStrictlyEqualTo(*m_polyfaceCacheDisplay))
        {
        FlushPolyfaceCache();
        m_polyfaceCache = IPolyfaceConstruction::Create(m_facetOptions);
        m_polyfaceCacheDisplay = &displayParams;
        }

    m_polyfaceCache->Add(*clone);
 
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometryProcessor::FlushPolyfaceCache ()
    {
    if (m_polyfaceCache.IsValid())
        {
        DRange3d range = m_polyfaceCache->GetClientMeshR().PointRange();

        IGeometryPtr geom = IGeometry::Create(m_polyfaceCache->GetClientMeshPtr());
        AddElementGeometry(*TileGeometry::Create(*geom, Transform::FromIdentity(), range, m_curElemId, *m_polyfaceCacheDisplay, false, false, m_dgndb));

        m_polyfaceCache = nullptr;
        m_polyfaceCacheDisplay = nullptr;
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessBody(IBRepEntityCR solid, SimplifyGraphic& gf) 
    {
    IBRepEntityPtr  clone = const_cast<IBRepEntityP>(&solid);
    DRange3d        range = clone->GetEntityRange();
    Transform       localToTile = Transform::FromProduct(m_transformFromDgn, gf.GetLocalToWorldTransform());

    localToTile.Multiply(range, range);

    TileDisplayParamsCR displayParams = m_displayParamsCache.Get(gf.GetCurrentGraphicParams(), gf.GetCurrentGeometryParams(), m_is2d);

    AddElementGeometry(*TileGeometry::Create(*clone, localToTile, range, m_curElemId, displayParams, m_dgndb));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessTextString(TextStringCR textString, SimplifyGraphic& gf) 
    {
    if (m_surfacesOnly)
        return true;

    static BeMutex s_tempFontMutex;
    BeMutexHolder lock(s_tempFontMutex);        // Temporary - until we resolve the font threading issues.

    TextStringPtr   clone = textString.Clone();
    Transform       localToTile = Transform::FromProduct(m_transformFromDgn, gf.GetLocalToWorldTransform());
    DRange2d        range2d   = clone->GetRange();
    DRange3d        range     = DRange3d::From (range2d.low.x, range2d.low.y, 0.0, range2d.high.x, range2d.high.y, 0.0);

    Transform::FromProduct (localToTile, clone->ComputeTransform()).Multiply (range, range);
                               
    TileDisplayParamsCR displayParams = m_displayParamsCache.Get(gf.GetCurrentGraphicParams(), gf.GetCurrentGeometryParams(), true /* Ignore lighting */);

    AddElementGeometry(*TileGeometry::Create(*clone, localToTile, range, m_curElemId, displayParams, m_dgndb));

    return true;
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
struct GeometryCollector : RangeIndex::Traverser
{
    TileGeometryProcessor&  m_processor;
    ViewContextR            m_context;
    RangeIndex::FBox        m_range;

    GeometryCollector(DRange3dCR range, TileGeometryProcessor& proc, ViewContextR context)
        : m_range(range), m_processor(proc), m_context(context) { }

    RangeIndex::Traverser::Accept _CheckRangeTreeNode(RangeIndex::FBoxCR box, bool is3d) const override
        {
        return box.IntersectsWith(m_range) ? RangeIndex::Traverser::Accept::Yes : RangeIndex::Traverser::Accept::No;
        }

    Stop _VisitRangeTreeEntry(RangeIndex::EntryCR entry) override
        {
        if (entry.m_range.IntersectsWith(m_range))
            {
            auto entryRange = entry.m_range.ToRange3d();
            if (!m_processor.BelowMinRange(entryRange))
                m_processor.ProcessElement(m_context, entry.m_id, entryRange);
            }

        return Stop::No;
        }

    TileGeneratorStatus Collect()
        {
        auto model = m_processor.GetCache().GetModel().ToGeometricModelP();
        if (nullptr == model || DgnDbStatus::Success != model->FillRangeIndex())
            return TileGeneratorStatus::NoGeometry;

        return Stop::Yes == model->GetRangeIndex()->Traverse(*this) ? TileGeneratorStatus::Aborted : TileGeneratorStatus::Success;
        }
};
         
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeneratorStatus TileGeometryProcessor::OutputGraphics(ViewContextR context)
    {
    GeometryCollector collector(m_range, *this, context);

    auto status = collector.Collect();
    if (TileGeneratorStatus::Aborted == status)
        {
        m_geometries.clear();
        }
    else if (TileGeneratorStatus::Success == status)
        {
        Sheet::ModelCP sheetModel = m_cache.GetModel().ToSheetModel();

        if (nullptr != sheetModel)
            {
            m_curElemId.Invalidate();
            Sheet::Model::DrawBorder (context, sheetModel->GetSheetSize());
            PushCurrentGeometry();
            }

        // We sort by size in order to ensure the largest geometries are assigned batch IDs
        // If the number of geometries does not exceed the max number of batch IDs, they will all get batch IDs so sorting is unnecessary
        if (m_geometries.size() > s_maxGeometryIdCount)
            {
            std::sort(m_geometries.begin(), m_geometries.end(), [&](TileGeometryPtr const& lhs, TileGeometryPtr const& rhs)
                {
                DRange3d lhsRange, rhsRange;
                lhsRange.IntersectionOf(lhs->GetTileRange(), m_range);
                rhsRange.IntersectionOf(rhs->GetTileRange(), m_range);
                return lhsRange.DiagonalDistance() < rhsRange.DiagonalDistance();
                });
            }
        }

    return status;
    }


//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
template<typename T> struct TileGeometryProcessorContext : NullContext
{
DEFINE_T_SUPER(NullContext);

private:
    TileGeometryProcessor&          m_processor;
    TileGenerationCacheCR           m_cache;
    BeSQLite::CachedStatementPtr    m_statement;

    bool IsValueNull(int index) { return m_statement->IsColumnNull(index); }

    Render::GraphicBuilderPtr _CreateGraphic(Render::GraphicBuilder::CreateParams const& params) override
        {
        return new SimplifyGraphic(params, m_processor, *this);
        }

    StatusInt _VisitElement(DgnElementId elementId, bool allowLoad) override;
    Render::GraphicPtr _StrokeGeometry(GeometrySourceCR, double) override;

    static Render::ViewFlags GetDefaultViewFlags()
        {
        // Ensure all classes/types of elements included...visibility can be controlled by declarative styling.
        // Most default to on.
        Render::ViewFlags flags;
        flags.SetShowConstructions(true);
        return flags;
        }
public:
    TileGeometryProcessorContext(TileGeometryProcessor& processor, DgnDbR db, TileGenerationCacheCR cache) : m_processor(processor), m_cache(cache),
    m_statement(db.GetCachedStatement(T::GetSql()))
        {
        SetDgnDb(db);
        m_is3dView = T::Is3d(); // force Brien to call _AddArc2d() if we're in a 2d model...
        SetViewFlags(GetDefaultViewFlags());
        }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void _AddSubGraphic(Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, GeometryParamsR geomParams) override
    {
    DgnGeometryPartCPtr     geomPart = m_processor.GetDgnDb().Elements().template Get<DgnGeometryPart>(partId);

    if (!geomPart.IsValid())
        {
        BeAssert(false);
        return;
        }

    static  size_t s_minInstancePartSize = 2000;

    if (geomPart->GetGeometryStream().size() > s_minInstancePartSize &&
        m_processor.IsGeomPartContained(graphic, *geomPart, subToGraphic) && 
        graphic.GetLocalToWorldTransform().Determinant() > 0.0)  // Mirroring...
        {
        GraphicParams graphicParams;
        _CookGeometryParams(geomParams, graphicParams);

        m_processor.AddGeomPart(graphic, *geomPart, subToGraphic, geomParams, graphicParams, *this);
        }
    else
        {
        T_Super::_AddSubGraphic(graphic, partId, subToGraphic, geomParams);
        }
    }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> StatusInt TileGeometryProcessorContext<T>::_VisitElement(DgnElementId elementId, bool allowLoad)
    {
    GeometrySourceCP pSrc = m_cache.GetCachedGeometrySource(elementId);
    if (nullptr != pSrc)
        return VisitGeometry(*pSrc);

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
    m_cache.GetDbMutex().Enter();
    StatusInt status = ERROR;
    auto& stmt = *m_statement;
    stmt.BindInt64(1, static_cast<int64_t>(elementId.GetValueUnchecked()));

    if (BeSQLite::BE_SQLITE_ROW == stmt.Step() && !IsValueNull(1))
        {
        auto geomSrcPtr = T::ExtractGeometrySource(stmt, GetDgnDb());

        stmt.Reset();
        m_cache.GetDbMutex().Leave();

        pSrc = m_cache.AddCachedGeometrySource(geomSrcPtr, elementId);

        if (nullptr != pSrc)
            status = VisitGeometry(*pSrc);
        }
    else
        {
        stmt.Reset();
        m_cache.GetDbMutex().Leave();
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> Render::GraphicPtr TileGeometryProcessorContext<T>::_StrokeGeometry(GeometrySourceCR source, double pixelSize)
    {
    Render::GraphicPtr graphic = source.Draw(*this, pixelSize);
    return WasAborted() ? nullptr : graphic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeneratorStatus ElementTileNode::_CollectGeometry(TileGenerationCacheCR cache, DgnDbR db, bool* leafThresholdExceeded, double tolerance, bool surfacesOnly, size_t leafCountThreshold)
    {
    // Collect geometry from elements in this node, sorted by size
    auto is2d = cache.GetModel().Is2dModel();
    IFacetOptionsPtr                facetOptions = TileGenerator::CreateTileFacetOptions(tolerance);
    TileGeometryProcessor           processor(m_geometries, cache, db, GetDgnRange(), *facetOptions, m_transformFromDgn, leafThresholdExceeded, tolerance, surfacesOnly, leafCountThreshold, is2d);


    if (is2d)
        {
        TileGeometryProcessorContext<GeometrySelector2d> context(processor, db, cache);

        return processor.OutputGraphics(context);
        }
    else
        {
        TileGeometryProcessorContext<GeometrySelector3d> context(processor, db, cache);
        return processor.OutputGraphics(context);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PublishableTileGeometry ElementTileNode::_GeneratePublishableGeometry(DgnDbR db, TileGeometry::NormalMode normalMode,  bool doSurfacesOnly, ITileGenerationFilterCP filter) const
    {
    bmap<TileGeomPartCP, TileMeshPartPtr>   partMap;
    TileGeometryList            uninstancedGeometry;
    PublishableTileGeometry     publishedTileGeometry;
    TileMeshList&               meshes = publishedTileGeometry.Meshes();
    size_t                      minInstanceCount = m_geometries.size() / 50;               // If the part will include 1/50th of geometry, do instancing (even if part does not deem it worthy).
    minInstanceCount = std::max(minInstanceCount, (size_t)2);

    // Extract instances first...
    for (auto& geom : m_geometries)
        {
        auto const&     part = geom->GetPart();
        static bool     s_disableInstancing = false;

        if (!s_disableInstancing && (part.IsValid() && (part->GetInstanceCount() > minInstanceCount || part->IsWorthInstancing(GetTolerance()))))
            {
            auto const&         found = partMap.find(part.get());
            TileMeshPartPtr     meshPart;

            if (found == partMap.end())
                {           
                TileMeshList    partMeshes = GenerateMeshes(db, normalMode, doSurfacesOnly, false, filter, part->GetGeometries());

                if (partMeshes.empty())
                    continue;
                
                for (auto& partMesh : partMeshes)
                    partMesh->SetValidIdsPresent(false);    // Ids are included on the instances only.

                publishedTileGeometry.Parts().push_back(meshPart = TileMeshPart::Create (std::move(partMeshes)));
                partMap.Insert(part.get(), meshPart);
                }
            else
                {
                meshPart = found->second;
                }

            meshPart->AddInstance (TileMeshInstance(geom->GetAttributes(), geom->GetTransform()));
            m_containsParts = true;
            }
        else
            {
            uninstancedGeometry.push_back(geom);
            }
        }
    TileMeshList    uninstancedMeshes = GenerateMeshes (db, normalMode, doSurfacesOnly, true, filter, uninstancedGeometry);

    meshes.insert (meshes.end(), uninstancedMeshes.begin(), uninstancedMeshes.end());

    return publishedTileGeometry;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileMeshList ElementTileNode::GenerateMeshes(DgnDbR db, TileGeometry::NormalMode normalMode, bool doSurfacesOnly, bool doRangeTest, ITileGenerationFilterCP filter, TileGeometryList const& geometries) const
    {
    static const double         s_vertexToleranceRatio    = .1;
    static const double         s_vertexClusterThresholdPixels = 5.0;
    static const double         s_facetAreaToleranceRatio = .1;
    static const size_t         s_decimatePolyfacePointCount = 100;

    double                      tolerance = GetTolerance();
    double                      vertexTolerance = tolerance * s_vertexToleranceRatio;
    double                      facetAreaTolerance   = tolerance * tolerance * s_facetAreaToleranceRatio;

    // Convert to meshes
    MeshBuilderMap      builderMap;
    DRange3d            myTileRange = GetTileRange();

    for (auto& geom : geometries)
        {
        if (nullptr != filter && !filter->AcceptElement(DgnElementId(geom->GetEntityId().GetValue())))
            continue;

        DRange3dCR  geomRange = geom->GetTileRange();
        double      rangePixels = geomRange.DiagonalDistance() / tolerance;

        if (rangePixels < s_minRangeBoxSize)
            continue;   // ###TODO: -- Produce an artifact from optimized bounding box to approximate from range.

        auto        polyfaces = geom->GetPolyfaces(tolerance, normalMode);
        bool        isContained = !doRangeTest || geomRange.IsContained(myTileRange);

        FeatureAttributes attributes = geom->GetAttributes();
        for (auto& tilePolyface : polyfaces)
            {
            TileDisplayParamsCPtr   displayParams = tilePolyface.m_displayParams;
            PolyfaceHeaderPtr       polyface = tilePolyface.m_polyface;
            bool                    hasTexture = displayParams.IsValid() && displayParams->HasTexture(db);  // Can't rely on geom.HasTexture - this may come from a face attachment to a B-Rep.

            if (0 == polyface->GetPointCount())
                continue;

            TileMeshMergeKey key(*displayParams, polyface.IsValid() && nullptr != polyface->GetNormalIndexCP(), polyface.IsValid(), geom->GetEntityId().IsValid());

            TileMeshBuilderPtr meshBuilder;
            auto found = builderMap.find(key);
            if (builderMap.end() != found)
                meshBuilder = found->second;
            else
                builderMap[key] = meshBuilder = TileMeshBuilder::Create(*displayParams, m_transformFromDgn, vertexTolerance, facetAreaTolerance, const_cast<FeatureAttributesMapR>(m_attributes));

            if (polyface.IsValid())
                {
                // Decimate if the range of the geometry is small in the tile OR we are not in a leaf and we have geometry originating from polyface with many points (railings from Penn state building).
                // A polyface with many points is likely a tesselation from an outside source.
                static bool s_forceVertexCluster = false;
                bool        doDecimate           = !m_isLeaf && geom->DoDecimate() && polyface->GetPointCount() > s_decimatePolyfacePointCount;
                bool        doVertexCluster      = s_forceVertexCluster || (doDecimate && geom->DoVertexCluster() && rangePixels < s_vertexClusterThresholdPixels);

                if (doDecimate)
                    polyface->DecimateByEdgeCollapse (tolerance, 0.0);

                for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*polyface); visitor->AdvanceToNextFace(); /**/)
                    {
                    if (isContained || myTileRange.IntersectsWith(DRange3d::From(visitor->GetPointCP(), static_cast<int32_t>(visitor->Point().size()))))
                        {
                        meshBuilder->AddTriangle (*visitor, displayParams->GetMaterialId(), db, attributes, doVertexCluster, hasTexture, hasTexture ? 0 : displayParams->GetColor());
                        }
                    }
                }
            }

        if (!doSurfacesOnly)
            {
            auto                tileStrokesArray = geom->GetStrokes(*geom->CreateFacetOptions (tolerance, TileGeometry::NormalMode::Never));
        
            for (auto& tileStrokes : tileStrokesArray)
                {
                TileDisplayParamsCPtr   displayParams = tileStrokes.m_displayParams;
                TileMeshMergeKey key(*displayParams, false, false, geom->GetEntityId().IsValid());

                TileMeshBuilderPtr meshBuilder;
                auto found = builderMap.find(key);
                if (builderMap.end() != found)
                    meshBuilder = found->second;
                else
                    builderMap[key] = meshBuilder = TileMeshBuilder::Create(*displayParams, m_transformFromDgn, vertexTolerance, facetAreaTolerance, const_cast<FeatureAttributesMapR>(m_attributes));

                for (auto& strokePoints : tileStrokes.m_strokes)
                    meshBuilder->AddPolyline (strokePoints, attributes, rangePixels < s_vertexClusterThresholdPixels, displayParams->GetColor());
                }
            }
        }

    TileMeshList meshes;
       
    for (auto& builder : builderMap)
        if (!builder.second->GetMesh()->IsEmpty())
            meshes.push_back (builder.second->GetMesh());

    return meshes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileUtil::WriteJsonToFile (WCharCP fileName, Json::Value const& value)
    {
    BeFile          outputFile;

    if (BeFileStatus::Success != outputFile.Create (fileName))
        return ERROR;
   
    Utf8String  string = Json::FastWriter().write(value);

    return BeFileStatus::Success == outputFile.Write (nullptr, string.data(), string.size()) ? SUCCESS : ERROR;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileUtil::ReadJsonFromFile (Json::Value& value, WCharCP fileName)
    {
    Json::Reader    reader;
    ByteStream      inputData;
    BeFile          inputFile;

    return BeFileStatus::Success == inputFile.Open (fileName, BeFileAccess::Read) &&
           BeFileStatus::Success == inputFile.ReadEntireFile (inputData) &&
           reader.parse ((char*) inputData.GetData(), (char*) (inputData.GetData() + inputData.GetSize()), value) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
WString TileUtil::GetRootNameForModel(DgnModelCR model)
    {
    WString name(model.GetName().c_str(), BentleyCharEncoding::Utf8);
    name.append(1, '_');
    WChar idBuf[17];
    BeStringUtilities::FormatUInt64(idBuf, _countof(idBuf), model.GetModelId().GetValue(), HexFormatOptions::None);
    name.append(idBuf);
    return name;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool FeatureAttributes::operator<(FeatureAttributesCR rhs) const
    {
    if (IsUndefined())
        return rhs.IsDefined();
    else if (rhs.IsUndefined())
        return false;
    else if (GetElementId() != rhs.GetElementId())
        return GetElementId() < rhs.GetElementId();
    else if (GetSubCategoryId() != rhs.GetSubCategoryId())
        return GetSubCategoryId() < rhs.GetSubCategoryId();
    else if (GetClass() != rhs.GetClass())
        return static_cast<uint8_t>(GetClass()) < static_cast<uint8_t>(rhs.GetClass());
    else
        return false;
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
FeatureAttributesMap::FeatureAttributesMap()
    {
    // 0 always maps to "no attributes defined"
    FeatureAttributes undefined;
    m_map[undefined] = 0;

    BeAssert(1 == GetNumIndices());
    BeAssert(0 == GetIndex(undefined));
    BeAssert(1 == GetNumIndices());
    BeAssert(!AnyDefined());
    BeAssert(!IsFull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
uint16_t FeatureAttributesMap::GetIndex(TileGeometryCR geom)
    {
    return GetIndex(geom.GetAttributes());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void FeatureAttributesMap::RemoveUndefined()
    {
    // Cesium's instanced models require that indices range from [0, nInstances). Must remove the "undefined" entry for that to work.
    BeAssert(AnyDefined());

    FeatureAttributes undefined;
    m_map.erase(undefined);

    for (auto& kvp : m_map)
        kvp.second -= 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileDisplayParamsCR TileDisplayParamsCache::Get(TileDisplayParamsR toFind)
    {
    toFind.AddRef();
    TileDisplayParamsCPtr pToFind(&toFind);
    auto iter = m_set.find(pToFind);
    if (m_set.end() == iter)
        iter = m_set.insert(toFind.Clone()).first;

    return **iter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileDisplayParamsCPtr TileDisplayParams::Clone() const
    {
    TileDisplayParamsPtr clone(new TileDisplayParams());
    clone->m_categoryId = m_categoryId;
    clone->m_subCategoryId = m_subCategoryId;
    clone->m_color = m_color;
    clone->m_rasterWidth = m_rasterWidth;
    clone->m_materialId = m_materialId;
    clone->m_class = m_class;
    clone->m_ignoreLighting = m_ignoreLighting;
    clone->m_textureImage = m_textureImage;
    clone->m_linePixels = m_linePixels;
    clone->m_isColorFromBackground = m_isColorFromBackground;
    if (m_gradient.IsValid())
        {
        clone->m_gradient = GradientSymb::Create();
        clone->m_gradient->CopyFrom(*m_gradient);
        }
    return clone;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static int compareValues(T const& lhs, T const& rhs)
    {
    return lhs == rhs ? 0 : (lhs < rhs ? -1 : 1);
    }

#define TEST_LESS_THAN(LHS, RHS) \
    { \
    int cmp = compareValues(LHS, RHS); \
    if (0 != cmp) \
        return cmp < 0; \
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileDisplayParams::IsStrictlyLessThan(TileDisplayParamsCR rhs) const
    {
    TEST_LESS_THAN(m_categoryId.GetValueUnchecked(), rhs.m_categoryId.GetValueUnchecked());
    TEST_LESS_THAN(m_subCategoryId.GetValueUnchecked(), rhs.m_subCategoryId.GetValueUnchecked());
    TEST_LESS_THAN(m_color, rhs.m_color);
    TEST_LESS_THAN(m_rasterWidth, rhs.m_rasterWidth);
    TEST_LESS_THAN(m_materialId.GetValueUnchecked(), rhs.m_materialId.GetValueUnchecked());
    TEST_LESS_THAN(m_textureImage.get(), rhs.m_textureImage.get());
    TEST_LESS_THAN(m_linePixels, rhs.m_linePixels);
    TEST_LESS_THAN(static_cast<uint32_t>(m_class), static_cast<uint32_t>(rhs.m_class));
    TEST_LESS_THAN(m_gradient.get(), rhs.m_gradient.get());

    if (m_ignoreLighting != rhs.m_ignoreLighting)
        return m_ignoreLighting;

    return false;
    }

#define TEST_EQUAL(MEMBER) if (MEMBER != rhs.MEMBER) return false

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileDisplayParams::IsStrictlyEqualTo(TileDisplayParamsCR rhs) const
    {
    TEST_EQUAL(m_categoryId.GetValueUnchecked());
    TEST_EQUAL(m_subCategoryId.GetValueUnchecked());
    TEST_EQUAL(m_color);
    TEST_EQUAL(m_rasterWidth);
    TEST_EQUAL(m_materialId.GetValueUnchecked());
    TEST_EQUAL(m_textureImage.get());
    TEST_EQUAL(m_ignoreLighting);
    TEST_EQUAL(m_class);
    TEST_EQUAL(m_linePixels);
    TEST_EQUAL(m_gradient);
    TEST_EQUAL(m_class);

    return true;
    }


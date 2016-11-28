/*--------------------------------------------------------------------------------------+                                                                                                                         m
|
|     $Source: DgnCore/MeshTile.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <folly/BeFolly.h>
#include <Geom/XYZRangeTree.h>
#if defined (BENTLEYCONFIG_PARASOLID) 
#include <DgnPlatform/DgnBRep/PSolidUtil.h>
#endif

#if defined(BENTLEYCONFIG_OS_WINDOWS)
#include <windows.h>
#endif

USING_NAMESPACE_BENTLEY_RENDER

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


static ITileGenerationProgressMonitor   s_defaultProgressMeter;
// unused - static UnconditionalTileGenerationFilter s_defaultFilter;

struct RangeTreeNode
{
#if defined(BENTLEYCONFIG_64BIT_HARDWARE)
    static void FreeAll(XYZRangeTreeRoot& tree) { }

    static void Add(XYZRangeTreeRoot& tree, DgnElementId elemId, DRange3dCR range)
        {
        tree.Add(reinterpret_cast<void*>(elemId.GetValueUnchecked()), range);
        }

    static DgnElementId GetElementId(XYZRangeTreeLeaf& leaf)
        {
        return DgnElementId(reinterpret_cast<uint64_t>(leaf.GetData()));
        }
#else
    DgnElementId    m_elementId;

    RangeTreeNode(DgnElementId elemId) : m_elementId(elemId) { }

    struct FreeLeafDataTreeHandler : XYZRangeTreeHandler
    {
        virtual bool ShouldContinueAfterLeaf(XYZRangeTreeRootP pRoot, XYZRangeTreeInteriorP pInterior, XYZRangeTreeLeafP pLeaf) override
            {
            delete reinterpret_cast<RangeTreeNode*>(pLeaf->GetData());
            return true;
            }
    };

    static void FreeAll(XYZRangeTreeRoot& tree)
        {
        FreeLeafDataTreeHandler handler;
        tree.Traverse(handler);
        }

    static void Add(XYZRangeTreeRoot& tree, DgnElementId elemId, DRange3dCR range)
        {
        tree.Add(new RangeTreeNode(elemId), range);
        }

    static DgnElementId GetElementId(XYZRangeTreeLeaf& leaf)
        {
        auto const& node = *reinterpret_cast<RangeTreeNode const*>(leaf.GetData());
        return node.m_elementId;
        }
#endif
};

static const int    s_splitCount         = 3;       // 3 splits per parent (oct-trees).
static const double s_minRangeBoxSize    = 0.5;     // Threshold below which we consider geometry/element too small to contribute to tile mesh
static const size_t s_maxGeometryIdCount = 0xffff;  // Max batch table ID - 16-bit unsigned integers
static const double s_minToleranceRatio = 100.0;

static Render::GraphicSet s_unusedDummyGraphicSet;

#if defined(MESHTILE_SELECT_GEOMETRY_USING_ECSQL)
static const Utf8CP s_geometrySource3dECSql = "SELECT CategoryId.Id,GeometryStream,Yaw,Pitch,Roll,Origin,BBoxLow,BBoxHigh FROM " BIS_SCHEMA(BIS_CLASS_GeometricElement3d) " WHERE ECInstanceId=?";
#else
static const Utf8CP s_geometrySource3dNativeSql =
    "SELECT CategoryId,GeometryStream,Yaw,Pitch,Roll,Origin_X,Origin_Y,Origin_Z,BBoxLow_X,BBoxLow_Y,BBoxLow_Z,BBoxHigh_X,BBoxHigh_Y,BBoxHigh_Z FROM "
    BIS_TABLE(BIS_CLASS_GeometricElement3d) " WHERE ElementId=?";
#endif

END_UNNAMED_NAMESPACE

#define COMPARE_VALUES_TOLERANCE(val0, val1, tol)   if (val0 < val1 - tol) return true; if (val0 > val1 + tol) return false;
#define COMPARE_VALUES(val0, val1) if (val0 < val1) { return true; } if (val0 > val1) { return false; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value     TileModelDelta::ElementState::GetJsonValue () const
    {
    Json::Value value(Json::objectValue);

    value["time"] = m_lastModifiedTime;
    value["count"] = m_facetCount;

    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TileModelDelta::Save()
    {
    Json::Value     value (Json::objectValue);

    for (auto& curr : m_elementStates)
        value[curr.first.ToString().c_str()] = curr.second.GetJsonValue();

    if (SUCCESS != TileUtil::WriteJsonToFile (m_fileName.c_str(), value))
        BeAssert (false && "Unable to open model state file");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileModelDelta::TileModelDelta (DgnModelCR model, BeFileNameCR dataDirectory) : m_dataDirectory (dataDirectory), m_rootName(TileUtil::GetRootNameForModel(model))
    {
    for (auto& it : model.MakeIterator())
        {
        int64_t         milliseconds;

        if (SUCCESS == it.GetLastModifyTime().ToUnixMilliseconds (milliseconds)) 
            m_elementStates.Insert (it.GetElementId(), ElementState(milliseconds, 0));
        }
    
    m_fileName = BeFileName (nullptr, dataDirectory.c_str(), L"ModelState", L"json");

    BeFile          inputFile;
    ByteStream      inputData;
    Json::Value     value;
    Json::Reader    reader;

    if (BeFileStatus::Success != inputFile.Open (m_fileName.c_str(), BeFileAccess::Read) ||
        BeFileStatus::Success != inputFile.ReadEntireFile (inputData) ||
        !reader.parse ((char*) inputData.GetData(), (char*) (inputData.GetData() + inputData.GetSize()), value))
        {
        for (auto& element : m_elementStates)
            m_added.insert (element.first);

        return;
        }


    auto    members = value.getMemberNames();

    for (auto& member : members)
        {
        DgnElementId    id;

        if (SUCCESS == BeInt64Id::FromString (id, member.c_str()))
            {
            auto const&     found = m_elementStates.find(id);
            Json::Value&    previousElementState = value[member.c_str()];
            int64_t         previousTime = previousElementState["time"].asInt64();
            int32_t         previousFacetCount = previousElementState["count"].asInt();

            if (found != m_elementStates.end() && previousTime == found->second.GetLastModifiedTime())
                found->second.SetFacetCount (previousFacetCount);
            else
                m_deleted.insert(id);
            }
        }
    for (auto& elementState : m_elementStates)
        if (0 == elementState.second.GetFacetCount())
            m_added.insert (elementState.first);

    TileUtil::ReadJsonFromFile (m_previousTileSet, BeFileName (nullptr, dataDirectory.c_str(), m_rootName.c_str(), L"json").c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus findTileValue (Json::Value& tile, Json::Value const& tileSet, bvector<size_t>& tilePath)
    {
    if (tilePath.empty())
        {
        tile = tileSet;
        return SUCCESS;
        }
    
    Json::Value     children = tileSet["children"];
    uint32_t        index = tilePath.back();

    if (children.isNull() || index >= children.size() )
        return ERROR;


    tilePath.pop_back();
    return findTileValue (tile, children[index], tilePath);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileModelDelta::DoIncremental (TileNodeCR tile) const
    {
    Json::Value     previousTile;
    BeFile          testFile;
    BeFileName      tileFileName (nullptr, m_dataDirectory.c_str(), tile.GetFileName (m_rootName.c_str(), L"b3dm").c_str(), nullptr);
    bvector<size_t> tilePath;

    for (TileNodeCP pathTile = &tile; nullptr != pathTile->GetParent(); pathTile = pathTile->GetParent())
        tilePath.push_back (pathTile->GetSiblingIndex());

    return SUCCESS == findTileValue (previousTile, m_previousTileSet["root"], tilePath) && 
           fabs(previousTile["geometricError"].asDouble() - tile.GetTolerance()) < 1.0E-8 &&
           BeFileStatus::Success == testFile.Open (tileFileName.c_str(), BeFileAccess::Read);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerationCache::TileGenerationCache(Options options) : m_tree(XYZRangeTreeRoot::Allocate()), m_options(options),
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
    RangeTreeNode::FreeAll(*m_tree);

    XYZRangeTreeRoot::Free(m_tree);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d TileGenerationCache::GetRange() const
    {
    return GetTree().Range();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGenerationCache::Populate(DgnDbR db, DgnModelId modelId)       
    {
    Statement stmt(db,  "SELECT ElementId,MinX,MinY,MinZ,MaxX,MaxY,MaxZ FROM " DGN_VTABLE_SpatialIndex " r, " BIS_TABLE(BIS_CLASS_Element) " e WHERE r.ElementId=e.Id AND e.ModelId=?");
    stmt.BindId(1, modelId);

    while (BE_SQLITE_ROW == stmt.Step())
        {
        auto elemId = stmt.GetValueId<DgnElementId>(0);
        DRange3d elRange = DRange3d::From(stmt.GetValueDouble(1), stmt.GetValueDouble(2), stmt.GetValueDouble(3),
                                          stmt.GetValueDouble(4), stmt.GetValueDouble(5), stmt.GetValueDouble(6));

        RangeTreeNode::Add(*m_tree, elemId, elRange);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureCPtr TileDisplayParams::QueryTexture(DgnDbR db) const
    {
    JsonRenderMaterial mat;
    if (!m_materialId.IsValid() || SUCCESS != mat.Load(m_materialId, db))
        return nullptr;

    auto texMap = mat.GetPatternMap();
    DgnTextureId texId;
    if (!texMap.IsValid() || !(texId = texMap.GetTextureId()).IsValid())
        return nullptr;

    return DgnTexture::Get(db, texId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileDisplayParams::TileDisplayParams(GraphicParamsCP graphicParams, GeometryParamsCP geometryParams, bool ignoreLighting) :
    m_fillColor(nullptr != graphicParams ? graphicParams->GetFillColor().GetValue() : 0x00ffffff), m_ignoreLighting (ignoreLighting)
    {
    if (nullptr != geometryParams)
        {
        m_categoryId = geometryParams->GetCategoryId();
        m_subCategoryId = geometryParams->GetSubCategoryId();
        m_materialId = geometryParams->GetMaterialId();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileDisplayParams::operator<(TileDisplayParams const& rhs) const
    {
    COMPARE_VALUES (m_fillColor, rhs.m_fillColor);
    COMPARE_VALUES (m_materialId.GetValueUnchecked(), rhs.m_materialId.GetValueUnchecked());

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
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileDisplayParams::ResolveTextureImage(DgnDbR db) const
    {
    if (m_textureImage.IsValid())
        return;

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void    TileMesh::AddMesh (TileMeshCR mesh)
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
        AddTriangle (TileTriangle (triangle.m_indices[0] + baseIndex, triangle.m_indices[1] + baseIndex, triangle.m_indices[2] + baseIndex, triangle.m_singleSided));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool    TileMesh::RemoveEntityGeometry (bset<DgnElementId> const& deleteIds)
    {
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
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t TileMesh::AddVertex(DPoint3dCR point, DVec3dCP normal, DPoint2dCP param, DgnElementId entityId)
    {
    auto index = static_cast<uint32_t>(m_points.size());

    m_points.push_back(point);
    m_entityIds.push_back(entityId);

    if (nullptr != normal)
        m_normals.push_back(*normal);
                                                                                                                 

    if (nullptr != param)
        m_uvParams.push_back(*param);

    m_validIdsPresent |= entityId.IsValid();
    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileMeshBuilder::VertexKey::Comparator::operator()(VertexKey const& lhs, VertexKey const& rhs) const
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
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileMeshBuilder::AddTriangle(PolyfaceVisitorR visitor, DgnMaterialId materialId, DgnDbR dgnDb, DgnElementId entityId, bool doVertexCluster, bool duplicateTwoSidedTriangles, bool includeParams)
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
        (m_material.IsValid() || (materialId.IsValid() && SUCCESS == m_material.Load (materialId, dgnDb))))
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
    ++m_triangleIndex;

    if (visitor.GetTwoSided() && duplicateTwoSidedTriangles)
        {
        TileTriangle dupTriangle(false);

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
        ++m_triangleIndex;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TileMeshBuilder::AddPolyline (bvector<DPoint3d>const& points, DgnElementId entityId, bool doVertexCluster)
    {
    TilePolyline    newPolyline;

    for (auto& point : points)
        {
        VertexKey vertex(point, nullptr, nullptr, entityId);

        newPolyline.m_indices.push_back (doVertexCluster ? AddClusteredVertex(vertex) : AddVertex(vertex));
        }
    m_mesh->AddPolyline (newPolyline);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TileMeshBuilder::AddPolyface (PolyfaceQueryCR polyface, DgnMaterialId materialId, DgnDbR dgnDb, DgnElementId entityId, bool twoSidedTriangles, bool includeParams)
    {
    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(polyface); visitor->AdvanceToNextFace(); )
        AddTriangle(*visitor, materialId, dgnDb, entityId, false, twoSidedTriangles, includeParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t TileMeshBuilder::AddVertex(VertexKey const& vertex)
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
uint32_t TileMeshBuilder::AddClusteredVertex(VertexKey const& vertex)
    {
    auto found = m_clusteredVertexMap.find(vertex);
    if (m_clusteredVertexMap.end() != found)
        return found->second;

    auto index = m_mesh->AddVertex(vertex.m_point, vertex.GetNormal(), vertex.GetParam(), vertex.m_entityId);
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
TileGeometry::TileGeometry(TransformCR tf, DRange3dCR range, DgnElementId entityId, TileDisplayParamsPtr& params, bool isCurved, DgnDbR db)
    : m_params(params), m_transform(tf), m_tileRange(range), m_entityId(entityId), m_isCurved(isCurved), m_facetCount(0), m_hasTexture(params.IsValid() && params->QueryTexture(db).IsValid())
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

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct PrimitiveTileGeometry : TileGeometry
{
private:
    IGeometryPtr        m_geometry;

    PrimitiveTileGeometry(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsPtr& params, bool isCurved, DgnDbR db)
        : TileGeometry(tf, range, elemId, params, isCurved, db), m_geometry(&geometry) { }

    virtual T_TilePolyfaces _GetPolyfaces(IFacetOptionsR facetOptions) override;
    virtual bool _DoDecimate () const override { return m_geometry->GetAsPolyfaceHeader().IsValid(); }
    virtual size_t _GetFacetCount(FacetCounter& counter) const override { return counter.GetFacetCount(*m_geometry); }
    virtual T_TileStrokes _GetStrokes (IFacetOptionsR facetOptions) override;
public:
    static TileGeometryPtr Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsPtr& params, bool isCurved, DgnDbR db)
        {
        return new PrimitiveTileGeometry(geometry, tf, range, elemId, params, isCurved, db);
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

    SolidKernelTileGeometry(IBRepEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsPtr& params, DgnDbR db)
        : TileGeometry(tf, range, elemId, params, BRepUtil::HasCurvedFaceOrEdge(solid), db), m_entity(&solid) { }

    virtual T_TilePolyfaces _GetPolyfaces(IFacetOptionsR facetOptions) override;
    virtual size_t _GetFacetCount(FacetCounter& counter) const override { return counter.GetFacetCount(*m_entity); }
public:
    static TileGeometryPtr Create(IBRepEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsPtr& params, DgnDbR db)
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

    TextStringTileGeometry(TextStringR text, TransformCR transform, DRange3dCR range, DgnElementId elemId, TileDisplayParamsPtr& params, DgnDbR db)
        : TileGeometry(transform, range, elemId, params, true, db), m_text(&text) 
        { 
        InitGlyphCurves();     // Should be able to defer this when font threaded ness is resolved.
        }

    virtual bool _DoVertexCluster() const override { return false; }

public:
    static TileGeometryPtr Create(TextStringR textString, TransformCR transform, DRange3dCR range, DgnElementId elemId, TileDisplayParamsPtr& params, DgnDbR db)
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
virtual T_TilePolyfaces _GetPolyfaces(IFacetOptionsR facetOptions) override
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

    if (polyface.IsValid())
        {
        polyface->Transform(GetTransform());
        polyfaces.push_back (TileGeometry::TilePolyface (*GetDisplayParams(), polyface));
        }

    return polyfaces;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
virtual T_TileStrokes _GetStrokes (IFacetOptionsR facetOptions) override
    {
    T_TileStrokes               strokes;

    if (DoGlyphBoxes(facetOptions))
        return strokes;

    InitGlyphCurves();

    bvector<bvector<DPoint3d>>  strokePoints;

    for (auto& glyphCurve : m_glyphCurves)
        if (!glyphCurve->IsAnyRegionType())
            collectCurveStrokes(strokePoints, *glyphCurve, facetOptions, GetTransform());

    if (!strokePoints.empty())
        strokes.push_back (TileGeometry::TileStrokes (*GetDisplayParams(), std::move(strokePoints)));

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
};


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryPtr TileGeometry::Create(TextStringR textString, TransformCR transform, DRange3dCR range, DgnElementId entityId, TileDisplayParamsPtr& params, DgnDbR db)
    {
    return TextStringTileGeometry::Create(textString, transform, range, entityId, params, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryPtr TileGeometry::Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId entityId, TileDisplayParamsPtr& params, bool isCurved, DgnDbR db)
    {
    return PrimitiveTileGeometry::Create(geometry, tf, range, entityId, params, isCurved, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryPtr TileGeometry::Create(IBRepEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId entityId, TileDisplayParamsPtr& params, DgnDbR db)
    {
    return SolidKernelTileGeometry::Create(solid, tf, range, entityId, params, db);
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
        return TileGeometry::T_TilePolyfaces (1, TileGeometry::TilePolyface (*GetDisplayParams(), polyface));
        }

    CurveVectorPtr      curveVector = m_geometry->GetAsCurveVector();

    if (curveVector.IsValid() && !curveVector->IsAnyRegionType())       // Non region curveVectors....
        return T_TilePolyfaces();

    IPolyfaceConstructionPtr polyfaceBuilder = IPolyfaceConstruction::Create(facetOptions);

    ISolidPrimitivePtr  solidPrimitive = curveVector.IsNull() ? m_geometry->GetAsISolidPrimitive() : nullptr;
    MSBsplineSurfacePtr bsplineSurface = solidPrimitive.IsNull() && curveVector.IsNull() ? m_geometry->GetAsMSBsplineSurface() : nullptr;

    if (curveVector.IsValid())
        polyfaceBuilder->AddRegion(*curveVector);
    else if (solidPrimitive.IsValid())
        polyfaceBuilder->AddSolidPrimitive(*solidPrimitive);
    else if (bsplineSurface.IsValid())
        polyfaceBuilder->Add(*bsplineSurface);

    polyface = polyfaceBuilder->GetClientMeshPtr();
    if (polyface.IsValid())
        polyface->Transform(GetTransform());

    return TileGeometry::T_TilePolyfaces (1, TileGeometry::TilePolyface (*GetDisplayParams(), polyface));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometry::T_TileStrokes PrimitiveTileGeometry::_GetStrokes (IFacetOptionsR facetOptions)
    {
    CurveVectorPtr      curveVector = m_geometry->GetAsCurveVector();
    T_TileStrokes       tileStrokes;

    if (curveVector.IsValid() && ! curveVector->IsAnyRegionType())
        {
        bvector<bvector<DPoint3d>>  strokePoints;

        collectCurveStrokes(strokePoints, *curveVector, facetOptions, GetTransform());

        if (!strokePoints.empty())
            tileStrokes.push_back (TileGeometry::TileStrokes (*GetDisplayParams(), std::move(strokePoints)));
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
        baseParams.SetCategoryId(GetDisplayParams()->GetCategoryId());
        baseParams.SetSubCategoryId(GetDisplayParams()->GetSubCategoryId());

        for (size_t i=0; i<polyfaces.size(); i++)
            {
            auto&   polyface = polyfaces[i];

            if (polyface->HasFacets())
                {
                GeometryParams faceParams;

                params[i].ToGeometryParams(faceParams, baseParams);

                TileDisplayParamsPtr displayParams = TileDisplayParams::Create (GetDisplayParams()->GetFillColor(), faceParams);

                tilePolyfaces.push_back (TileGeometry::TilePolyface (*displayParams, polyface));
                }
            }
        }
    else
        {
        auto polyface = BRepUtil::FacetEntity(*m_entity, *pFacetOptions);
    
        if (polyface.IsValid() && polyface->HasFacets())
            tilePolyfaces.push_back (TileGeometry::TilePolyface (*GetDisplayParams(), polyface));

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
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::TileGenerator(TransformCR transformFromDgn, DgnDbR dgndb, ITileGenerationFilterP filter, ITileGenerationProgressMonitorP progress)
    : m_progressMeter(nullptr != progress ? *progress : s_defaultProgressMeter), m_transformFromDgn(transformFromDgn), m_dgndb(dgndb), 
      m_totalTiles(0), m_totalModels(0), m_completedModels(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::Status TileGenerator::GenerateTiles(ITileCollector& collector, DgnModelIdSet const& modelIds, double leafTolerance, size_t maxPointsPerTile)
    {
    auto nModels = static_cast<uint32_t>(modelIds.size());
    if (0 == nModels)
        return Status::NoGeometry;

    // unused - auto nCompletedModels = 0;

    T_HOST.GetFontAdmin().EnsureInitialized();
    GetDgnDb().Fonts().Update();

#if defined (BENTLEYCONFIG_PARASOLID) 
    ThreadedLocalParasolidHandlerStorageMark  parasolidParasolidHandlerStorageMark;
    PSolidKernelManager::StartSession();
#endif

    StopWatch timer(true);

    m_totalModels = nModels;
    m_progressMeter._IndicateProgress(0, nModels);

    auto future = GenerateTilesFromModels(collector, modelIds, leafTolerance, maxPointsPerTile);
    /* unused - auto status = */ future.get();

    m_completedModels.store(0);
    m_totalModels = 0;

    m_statistics.m_tileGenerationTime = timer.GetCurrentSeconds();
    m_statistics.m_tileCount = m_totalTiles;
    m_totalTiles.store(0);

    m_progressMeter._IndicateProgress(nModels, nModels);

    return Status::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::FutureStatus TileGenerator::GenerateTilesFromModels(ITileCollector& collector, DgnModelIdSet const& modelIds, double leafTolerance, size_t maxPointsPerTile)
    {
    std::vector<FutureStatus> modelFutures;
    for (auto const& modelId : modelIds)
        {
        auto model = GetDgnDb().Models().GetModel(modelId);
        if (model.IsValid())
            modelFutures.push_back(GenerateTiles(collector, leafTolerance, maxPointsPerTile, *model));
        }

    return folly::unorderedReduce(modelFutures, Status::Success, [=](Status reduced, Status next)
        {
        return Status::Aborted == reduced || Status::Aborted == next ? Status::Aborted : Status::Success;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::FutureStatus TileGenerator::GenerateTiles(ITileCollector& collector, double leafTolerance, size_t maxPointsPerTile, DgnModelR model)
    {
    DgnModelPtr     modelPtr(&model);
    auto            pCollector = &collector;
    auto            host = &T_HOST;
    auto            generateMeshTiles = dynamic_cast<IGenerateMeshTiles*>(&model);

    if (nullptr != generateMeshTiles)
        {
        return folly::via(&BeFolly::ThreadPool::GetIoPool(), [=]()
            {
            auto status = pCollector->_BeginProcessModel(*modelPtr);
            TileNodePtr root;
            if (Status::Success == status)
                {
                if (root.IsValid())
                    m_totalTiles += root->GetNodeCount();

                status = generateMeshTiles->_GenerateMeshTiles(root, m_transformFromDgn, *pCollector, GetProgressMeter());
                }

            m_progressMeter._IndicateProgress(++m_completedModels, m_totalModels);
            return pCollector->_EndProcessModel(*modelPtr, root.get(), status);
            });
        }
    else
        {
        BeFileName          dataDirectory;
        TileModelDeltaPtr   modelDelta;

        if (pCollector->_DoIncrementalModelPublish (dataDirectory, *modelPtr))
            modelDelta = TileModelDelta::Create (model, dataDirectory);

        return folly::via(&BeFolly::ThreadPool::GetIoPool(), [=]()
            {
            return pCollector->_BeginProcessModel(*modelPtr);
            })
        .then([=](Status status)
            {
            if (Status::Success == status)
                return GenerateElementTiles(*pCollector, leafTolerance, maxPointsPerTile, *modelPtr, modelDelta.get());

            return folly::makeFuture(ElementTileResult(status, nullptr));
            })
        .then([=](ElementTileResult result)
            {
            if (result.m_tile.IsValid())
                m_totalTiles += result.m_tile->GetNodeCount();

            m_progressMeter._IndicateProgress(++m_completedModels, m_totalModels);
            return pCollector->_EndProcessModel(*modelPtr, result.m_tile.get(), result.m_status);
            })
        .then([=](TileGenerator::Status status)
            {
            if (modelDelta.IsValid())
                modelDelta->Save ();

            return status;
            });
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::FutureElementTileResult TileGenerator::GenerateElementTiles(ITileCollector& collector, double leafTolerance, size_t maxPointsPerTile, DgnModelR model, TileModelDeltaP modelDelta)
    {
    auto                cache = TileGenerationCache::Create(TileGenerationCache::Options::CacheGeometrySources);
    ElementTileContext  context(*cache, model, modelDelta, collector, leafTolerance, maxPointsPerTile);

    return PopulateCache(context).then([=](TileGenerator::Status status)
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
    #if defined (BENTLEYCONFIG_PARASOLID) 
        ThreadedParasolidErrorHandlerOuterMarkPtr  outerMark = ThreadedParasolidErrorHandlerOuterMark::Create();
        ThreadedParasolidErrorHandlerInnerMarkPtr  innerMark = ThreadedParasolidErrorHandlerInnerMark::Create(); 
    #endif

        context.m_cache->Populate(GetDgnDb(), context.m_model->GetModelId());
        bool emptyRange = context.m_cache->GetRange().IsNull();
        return emptyRange ? Status::NoGeometry : Status::Success;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::FutureElementTileResult TileGenerator::GenerateTileset(Status status, ElementTileContext context)
    {
    auto& cache = *context.m_cache;
    if (Status::Success != status)
        {
        ElementTileResult result(status, ElementTileNode::Create(*context.m_model, context.m_modelDelta, cache.GetRange(), GetTransformFromDgn(), 0, 0, nullptr).get());
        return folly::makeFuture(result);
        }

    ElementTileNodePtr parent = ElementTileNode::Create(*context.m_model, context.m_modelDelta, cache.GetRange(), GetTransformFromDgn(), 0, 0, nullptr);
    return ProcessParentTile(parent, context).then([=](ElementTileResult result) { return ProcessChildTiles(result.m_status, parent, context); });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::FutureElementTileResult TileGenerator::ProcessParentTile(ElementTileNodePtr parent, ElementTileContext context)
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
        bool            isLeaf = tileTolerance < leafTolerance;
        bool            leafThresholdExceeded = false;

        // Always collect geometry at the target leaf tolerance.
        // If maxPointsPerTile is exceeded, we will keep that geometry, but adjust this tile's target tolerance
        // Later that tolerance will be used in _GenerateMeshes() to facet appropriately (and to filter out 
        // elements too small to be included in this tile)
        tile.CollectGeometry(generationCache, m_dgndb, context.m_modelDelta, &leafThresholdExceeded, leafTolerance, isLeaf ? 0 : maxPointsPerTile);

        if (!isLeaf && !leafThresholdExceeded)
            isLeaf = true;

        ElementTileResult result(m_progressMeter._WasAborted() ? Status::Aborted : Status::Success, static_cast<ElementTileNodeP>(tile.GetRoot()));
        if (tile.GetGeometries().empty() && nullptr == tile.GetModelDelta())
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

        size_t              siblingIndex = 0;
        bvector<DRange3d>   subRanges;

        tile.ComputeChildTileRanges(subRanges, tile.GetDgnRange(), s_splitCount);
        for (auto& subRange : subRanges)
            {
            ElementTileNodePtr child = ElementTileNode::Create(tile.GetModel(), tile.GetModelDelta(), subRange, m_transformFromDgn, tile.GetDepth()+1, siblingIndex++, &tile);

            tile.GetChildren().push_back(child);
            }

        tile.AdjustTolerance(tileTolerance);

        collector._AcceptTile(tile);
        tile.ClearGeometry();

        result.m_status = m_progressMeter._WasAborted() ? Status::Aborted : Status::Success;
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
TileGenerator::FutureElementTileResult TileGenerator::ProcessChildTiles(Status status, ElementTileNodePtr parent, ElementTileContext context)
    {
#if defined (BENTLEYCONFIG_PARASOLID) 
    ThreadedParasolidErrorHandlerOuterMarkPtr  outerMark = ThreadedParasolidErrorHandlerOuterMark::Create();
    ThreadedParasolidErrorHandlerInnerMarkPtr  innerMark = ThreadedParasolidErrorHandlerInnerMark::Create(); 
#endif

    auto root = static_cast<ElementTileNodeP>(parent->GetRoot());
    if (parent->GetChildren().empty() || Status::Success != status)
        return folly::makeFuture(ElementTileResult(status, root));

    std::vector<FutureElementTileResult> childFutures;
    for (auto& child : parent->GetChildren())
        {
        auto elemChild = static_cast<ElementTileNodeP>(child.get());
        auto childFuture = ProcessParentTile(elemChild, context).then([=](ElementTileResult result) { return ProcessChildTiles(result.m_status, elemChild, context); });
        childFutures.push_back(std::move(childFuture));
        }

    auto result = ElementTileResult(status, root);
    return folly::unorderedReduce(childFutures, result, [=](ElementTileResult, ElementTileResult)
        {
        return ElementTileResult(m_progressMeter._WasAborted() ? Status::Aborted : Status::Success, root);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementTileNode::ComputeChildTileRanges(bvector<DRange3d>& subRanges, DRange3dCR range, size_t splitCount)
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
#if defined(MESHTILE_SELECT_GEOMETRY_USING_ECSQL)
            m_blob = stmt.GetValueBinary(columnIndex, &m_size);
#else
            m_blob = stmt.GetValueBlob(columnIndex);
            m_size = stmt.GetColumnBytes(columnIndex);
#endif
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


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct TileGeometryProcessorContext : NullContext
{
private:
    IGeometryProcessorR     m_processor;
    TileGenerationCacheCR   m_cache;


#if defined(MESHTILE_SELECT_GEOMETRY_USING_ECSQL)
    BeSQLite::EC::CachedECSqlStatementPtr   m_statement;

    bool IsValueNull(int index) { return m_statement->IsValueNull(index); }
#else
    BeSQLite::CachedStatementPtr            m_statement;

    bool IsValueNull(int index) { return m_statement->IsColumnNull(index); }
#endif

    virtual Render::GraphicBuilderPtr _CreateGraphic(Render::Graphic::CreateParams const& params) override
        {
        return new SimplifyGraphic(params, m_processor, *this);
        }

    virtual StatusInt _VisitElement(DgnElementId elementId, bool allowLoad) override;
    virtual Render::GraphicPtr _StrokeGeometry(GeometrySourceCR, double) override;
public:
    TileGeometryProcessorContext(IGeometryProcessorR processor, DgnDbR db, TileGenerationCacheCR cache) : m_processor(processor), m_cache(cache),
#if defined(MESHTILE_SELECT_GEOMETRY_USING_ECSQL)
    m_statement(db.GetPreparedECSqlStatement(s_geometrySource3dECSql))
#else
    m_statement(db.GetCachedStatement(s_geometrySource3dNativeSql))
#endif
        {
        SetDgnDb(db);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt TileGeometryProcessorContext::_VisitElement(DgnElementId elementId, bool allowLoad)
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
        auto categoryId = stmt.GetValueId<DgnCategoryId>(0);
        TileGeometrySource::GeomBlob geomBlob(stmt, 1);

#if defined(MESHTILE_SELECT_GEOMETRY_USING_ECSQL)
        DPoint3d origin = stmt.GetValuePoint3d(5),
                 boxLo  = stmt.GetValuePoint3d(6),
                 boxHi  = stmt.GetValuePoint3d(7);
#else
        DPoint3d origin = DPoint3d::From(stmt.GetValueDouble(5), stmt.GetValueDouble(6), stmt.GetValueDouble(7)),
                 boxLo  = DPoint3d::From(stmt.GetValueDouble(8), stmt.GetValueDouble(9), stmt.GetValueDouble(10)),
                 boxHi  = DPoint3d::From(stmt.GetValueDouble(11), stmt.GetValueDouble(12), stmt.GetValueDouble(13));
#endif

        Placement3d placement(origin,
                YawPitchRollAngles(Angle::FromDegrees(stmt.GetValueDouble(2)), Angle::FromDegrees(stmt.GetValueDouble(3)), Angle::FromDegrees(stmt.GetValueDouble(4))),
                ElementAlignedBox3d(boxLo.x, boxLo.y, boxLo.z, boxHi.x, boxHi.y, boxHi.z));

        auto geomSrcPtr = TileGeometrySource3d::Create(categoryId, GetDgnDb(), geomBlob, placement);

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
Render::GraphicPtr TileGeometryProcessorContext::_StrokeGeometry(GeometrySourceCR source, double pixelSize)
    {
    Render::GraphicPtr graphic = source.Draw(*this, pixelSize);
    return WasAborted() ? nullptr : graphic;
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct TileGeometryProcessor : IGeometryProcessor
{
private:
    IFacetOptionsR              m_facetOptions;
    IFacetOptionsPtr            m_targetFacetOptions;
    DgnElementId                m_curElemId;
    TileGenerationCacheCR       m_cache;
    DgnDbR                      m_dgndb;
    TileGeometryList&           m_geometries;
    DRange3d                    m_range;
    DRange3d                    m_tileRange;
    Transform                   m_transformFromDgn;
    TileGeometryList            m_curElemGeometries;
    TileModelDeltaP             m_modelDelta;
    double                      m_minRangeDiagonal;
    double                      m_minTextBoxSize;
    bool*                       m_leafThresholdExceeded;
    size_t                      m_leafCountThreshold;
    size_t                      m_leafCount;


    void PushGeometry(TileGeometryR geom);
    void AddElementGeometry(TileGeometryR geom);
    bool ProcessGeometry(IGeometryR geometry, bool isCurved, SimplifyGraphic& gf);

    virtual IFacetOptionsP _GetFacetOptionsP() override { return &m_facetOptions; }

    virtual bool _ProcessCurveVector(CurveVectorCR curves, bool filled, SimplifyGraphic& gf) override;
    virtual bool _ProcessSolidPrimitive(ISolidPrimitiveCR prim, SimplifyGraphic& gf) override;
    virtual bool _ProcessSurface(MSBsplineSurfaceCR surface, SimplifyGraphic& gf) override;
    virtual bool _ProcessPolyface(PolyfaceQueryCR polyface, bool filled, SimplifyGraphic& gf) override;
    virtual bool _ProcessBody(IBRepEntityCR solid, SimplifyGraphic& gf) override;
    virtual bool _ProcessTextString(TextStringCR, SimplifyGraphic&) override;


    virtual UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR, SimplifyGraphic&) const override { return UnhandledPreference::Facet; }
    virtual UnhandledPreference _GetUnhandledPreference(CurveVectorCR, SimplifyGraphic&)     const override { return UnhandledPreference::Facet; }
    virtual UnhandledPreference _GetUnhandledPreference(IBRepEntityCR, SimplifyGraphic&)     const override { return UnhandledPreference::Facet; }

public:
    TileGeometryProcessor(TileGeometryList& geometries, TileGenerationCacheCR cache, DgnDbR db, DRange3dCR range, IFacetOptionsR facetOptions, TransformCR transformFromDgn, TileModelDeltaP modelDelta, bool* leafThresholdExceeded, double tolerance, size_t leafCountThreshold) 
        : m_geometries (geometries), m_facetOptions(facetOptions), m_targetFacetOptions(facetOptions.Clone()), m_cache(cache), m_dgndb(db), m_range(range), m_transformFromDgn(transformFromDgn), m_modelDelta(modelDelta),
          m_leafThresholdExceeded(leafThresholdExceeded), m_leafCountThreshold(leafCountThreshold), m_leafCount(0)
        {
        static const double s_minTextBoxSize = 1.0;     // Below this ratio to tolerance  text is rendered as box.

        m_targetFacetOptions->SetChordTolerance(facetOptions.GetChordTolerance() * transformFromDgn.ColumnXMagnitude());
        m_minRangeDiagonal = s_minRangeBoxSize * tolerance;
        m_minTextBoxSize  = s_minTextBoxSize * tolerance;
        m_transformFromDgn.Multiply (m_tileRange, m_range);
        }

    void ProcessElement(ViewContextR context, DgnElementId elementId, DRange3dCR range);
    virtual void _OutputGraphics(ViewContextR context) override;

    bool BelowMinRange(DRange3dCR range) const
        {
        // Avoid processing any elements with range smaller than roughly half a pixel...
        return range.DiagonalDistance() < m_minRangeDiagonal;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Ray.Bentley     11/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual UnhandledPreference _GetUnhandledPreference(TextStringCR textString, SimplifyGraphic& simplifyGraphic) const override 
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
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometryProcessor::ProcessElement(ViewContextR context, DgnElementId elemId, DRange3dCR dgnRange)
    {
    try
        {
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
        m_curElemGeometries.clear();
        bool haveCached = m_cache.GetCachedGeometry(m_curElemGeometries, elemId);
        if (!haveCached)
            {
            m_curElemId = elemId;
            context.VisitElement(elemId, false);
            }
        for (auto& geom : m_curElemGeometries)
            PushGeometry(*geom);

        if (!haveCached)
            {
            if (nullptr != elementState && 0 == elementState->GetFacetCount())    
                for (auto& geom : m_curElemGeometries)
                    elementState->SetFacetCount(elementState->GetFacetCount() + geom->GetFacetCount(*m_targetFacetOptions));

            m_cache.AddCachedGeometry(elemId, std::move(m_curElemGeometries));
            }
        }
    catch (...)
        {
        // This shouldn't be necessary - but an uncaught interception will cause the processing to continue forever. (OpenCascade error in LargeHatchPlant.)
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::ProcessGeometry(IGeometryR geom, bool isCurved, SimplifyGraphic& gf)
    {
    DRange3d range;
    if (!geom.TryGetRange(range))
        return false;   // ignore and continue

    auto tf = Transform::FromProduct(m_transformFromDgn, gf.GetLocalToWorldTransform());
    tf.Multiply(range, range);
    
    TileDisplayParamsPtr displayParams = TileDisplayParams::Create(gf.GetCurrentGraphicParams(), gf.GetCurrentGeometryParams());

    AddElementGeometry(*TileGeometry::Create(geom, tf, range, m_curElemId, displayParams, isCurved, m_dgndb));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessCurveVector(CurveVectorCR curves, bool filled, SimplifyGraphic& gf)
    {
    if (curves.IsAnyRegionType() && !curves.ContainsNonLinearPrimitive())
        return false;   // process as facets.

    CurveVectorPtr clone = curves.Clone();
    IGeometryPtr geom = IGeometry::Create(clone);
    return ProcessGeometry(*geom, false, gf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessSolidPrimitive(ISolidPrimitiveCR prim, SimplifyGraphic& gf) 
    {
    bool hasCurvedFaceOrEdge = prim.HasCurvedFaceOrEdge();
    if (!hasCurvedFaceOrEdge)
        return false;   // Process as facets.

    ISolidPrimitivePtr clone = prim.Clone();
    IGeometryPtr geom = IGeometry::Create(clone);
    return ProcessGeometry(*geom, hasCurvedFaceOrEdge, gf);
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
    return ProcessGeometry(*geom, isCurved, gf);
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

    DRange3d range = clone->PointRange();

    TileDisplayParamsPtr displayParams = TileDisplayParams::Create(gf.GetCurrentGraphicParams(), gf.GetCurrentGeometryParams());

    IGeometryPtr geom = IGeometry::Create(clone);
    AddElementGeometry(*TileGeometry::Create(*geom, Transform::FromIdentity(), range, m_curElemId, displayParams, false, m_dgndb));

    return true;
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

    TileDisplayParamsPtr displayParams = TileDisplayParams::Create(gf.GetCurrentGraphicParams(), gf.GetCurrentGeometryParams());

    AddElementGeometry(*TileGeometry::Create(*clone, localToTile, range, m_curElemId, displayParams, m_dgndb));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessTextString(TextStringCR textString, SimplifyGraphic& gf) 
    {
    static BeMutex s_tempFontMutex;
    BeMutexHolder lock(s_tempFontMutex);        // Temporary - until we resolve the font threading issues.

    TextStringPtr   clone = textString.Clone();
    Transform       localToTile = Transform::FromProduct(m_transformFromDgn, gf.GetLocalToWorldTransform());
    DRange2d        range2d   = clone->GetRange();
    DRange3d        range     = DRange3d::From (range2d.low.x, range2d.low.y, 0.0, range2d.high.x, range2d.high.y, 0.0);

    Transform::FromProduct (localToTile, clone->ComputeTransform()).Multiply (range, range);
                               
    TileDisplayParamsPtr displayParams = TileDisplayParams::Create(gf.GetCurrentGraphicParams(), gf.GetCurrentGeometryParams(), true /* Ignore lighting */);

    AddElementGeometry(*TileGeometry::Create(*clone, localToTile, range, m_curElemId, displayParams, m_dgndb));

    return true;

    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct GatherGeometryHandler : XYZRangeTreeHandler
{
    TileGeometryProcessor&  m_processor;
    ViewContextR            m_context;
    DRange3d                m_range;
    double                  m_tolerance;

    GatherGeometryHandler(DRange3dCR range, TileGeometryProcessor& proc, ViewContextR viewContext)
        : m_range(range), m_processor(proc), m_context(viewContext) { }

    virtual bool ShouldRecurseIntoSubtree(XYZRangeTreeRootP, XYZRangeTreeInteriorP pInterior) override
        {
        return pInterior->Range().IntersectsWith(m_range);
        }
    virtual bool ShouldContinueAfterLeaf(XYZRangeTreeRootP, XYZRangeTreeInteriorP pInterior, XYZRangeTreeLeafP pLeaf) override
        {
        if (pLeaf->Range().IntersectsWith(m_range) && !m_processor.BelowMinRange(pLeaf->Range()))
            m_processor.ProcessElement(m_context, RangeTreeNode::GetElementId(*pLeaf), pLeaf->Range());

        return true;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometryProcessor::_OutputGraphics(ViewContextR context)
    {
    GatherGeometryHandler handler(m_range, *this, context);
    m_cache.GetTree().Traverse(handler);

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementTileNode::_CollectGeometry(TileGenerationCacheCR cache, DgnDbR db, TileModelDeltaP modelDelta, bool* leafThresholdExceeded, double tolerance, size_t leafCountThreshold)
    {
    // Collect geometry from elements in this node, sorted by size
    IFacetOptionsPtr                facetOptions = createTileFacetOptions(tolerance);
    TileGeometryProcessor           processor(m_geometries, cache, db, GetDgnRange(), *facetOptions, m_transformFromDgn, modelDelta, leafThresholdExceeded, tolerance, leafCountThreshold);
    TileGeometryProcessorContext    context(processor, db, cache);

    processor._OutputGraphics(context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileMeshList ElementTileNode::_GenerateMeshes(DgnDbR db, TileGeometry::NormalMode normalMode, bool twoSidedTriangles, bool doPolylines, ITileGenerationFilterCP filter) const
    {
    static const double s_vertexToleranceRatio    = .1;
    static const double s_vertexClusterThresholdPixels = 5.0;
    static const double s_facetAreaToleranceRatio = .1;
    static const size_t s_decimatePolyfacePointCount = 100;

    double          tolerance = GetTolerance();
    double          vertexTolerance = tolerance * s_vertexToleranceRatio;
    double          facetAreaTolerance   = tolerance * tolerance * s_facetAreaToleranceRatio;

    // Convert to meshes
    MeshBuilderMap  builderMap;
    size_t          geometryCount = 0;
    DRange3d        myTileRange = GetTileRange();

    for (auto& geom : m_geometries)
        {
        if (nullptr != filter && 
            !filter->AcceptElement(DgnElementId(geom->GetEntityId().GetValue())))
            continue;

        DRange3dCR  geomRange = geom->GetTileRange();
        double      rangePixels = geomRange.DiagonalDistance() / tolerance;

        if (rangePixels < s_minRangeBoxSize)
            continue;   // ###TODO: -- Produce an artifact from optimized bounding box to approximate from range.

        auto        polyfaces = geom->GetPolyfaces(tolerance, normalMode);
        bool        isContained = geomRange.IsContained(myTileRange);
        bool        maxGeometryCountExceeded = (++geometryCount > s_maxGeometryIdCount);

        for (auto& tilePolyface : polyfaces)
            {
            TileDisplayParamsPtr    displayParams = tilePolyface.m_displayParams;
            PolyfaceHeaderPtr       polyface = tilePolyface.m_polyface;
            bool                    hasTexture = displayParams.IsValid() && displayParams->QueryTexture(db).IsValid();  // Can't rely on geom.HasTexture - this may come from a face attachment to a B-Rep.

            if (0 == polyface->GetPointCount())
                continue;

            TileMeshMergeKey key(*displayParams, polyface.IsValid() && nullptr != polyface->GetNormalIndexCP(), polyface.IsValid());

            TileMeshBuilderPtr meshBuilder;
            auto found = builderMap.find(key);
            if (builderMap.end() != found)
                meshBuilder = found->second;
            else
                builderMap[key] = meshBuilder = TileMeshBuilder::Create(displayParams, vertexTolerance, facetAreaTolerance);

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

                        meshBuilder->AddTriangle (*visitor, displayParams->GetMaterialId(), db, elemId, doVertexCluster, twoSidedTriangles, hasTexture);
                        }
                    }
                }
            }
        if (doPolylines)
            {
            auto                tileStrokesArray = geom->GetStrokes(*geom->CreateFacetOptions (tolerance, TileGeometry::NormalMode::Never));
        
            for (auto& tileStrokes : tileStrokesArray)
                {
                TileDisplayParamsPtr    displayParams = tileStrokes.m_displayParams;
                TileMeshMergeKey key(*displayParams, false, false);

                TileMeshBuilderPtr meshBuilder;
                auto found = builderMap.find(key);
                if (builderMap.end() != found)
                    meshBuilder = found->second;
                else
                    builderMap[key] = meshBuilder = TileMeshBuilder::Create(displayParams, vertexTolerance, facetAreaTolerance);


                DgnElementId elemId;
                if (geometryCount < s_maxGeometryIdCount)
                    elemId = geom->GetEntityId();

                for (auto& strokePoints : tileStrokes.m_strokes)
                    meshBuilder->AddPolyline (strokePoints, elemId, rangePixels < s_vertexClusterThresholdPixels);
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
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileModelCategoryFilter::TileModelCategoryFilter(DgnDbR db, DgnModelIdSet const* models, DgnCategoryIdSet const* categories) : m_set(models, categories)
    {
    static const Utf8CP s_sql = "SELECT g.ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_GeometricElement3d) " As g, " BIS_SCHEMA(BIS_CLASS_Element) " AS e "
                                " WHERE g.ECInstanceId=e.ECInstanceId AND InVirtualSet(?,e.ModelId.Id,g.CategoryId.Id)";

    m_stmt = db.GetPreparedECSqlStatement(s_sql);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileModelCategoryFilter::_AcceptElement(DgnElementId elementId) const
    {
    m_stmt->BindVirtualSet(1, m_set);
    bool accepted = BE_SQLITE_ROW == m_stmt->Step();
    m_stmt->Reset();
    return accepted;
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
    static const WString s_prefix(L"Model_");
    return s_prefix + WString(model.GetName().c_str(), BentleyCharEncoding::Utf8);
    }

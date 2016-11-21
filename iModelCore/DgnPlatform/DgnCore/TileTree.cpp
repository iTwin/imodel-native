/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/TileTree.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <folly/BeFolly.h>

USING_NAMESPACE_TILETREE

#define TABLE_NAME_TileTree "TileTree"

BEGIN_UNNAMED_NAMESPACE

//=======================================================================================
// Manage the creation and cleanup of the local TileCache used by TileData
// @bsiclass                                                    Keith.Bentley   08/16
//=======================================================================================
struct TileCache : RealityData::Cache
{
    uint64_t m_allowedSize;
    virtual BentleyStatus _Prepare() const override;
    virtual BentleyStatus _Cleanup() const override;
    TileCache(uint64_t maxSize) : m_allowedSize(maxSize) {}
};

DEFINE_REF_COUNTED_PTR(TileCache)

END_UNNAMED_NAMESPACE

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
BentleyStatus TileLoader::LoadTile()
    {
    // During the read we may have abandoned the tile. Do not waste time loading it.
    if (m_tile->IsAbandoned())
        return ERROR;

    BeAssert(m_tile->IsQueued());
    return _LoadTile();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
folly::Future<BentleyStatus> TileLoader::CreateTile()
    {
    if (m_loads != nullptr && m_loads->IsCanceled())
        {
        m_tile->SetNotLoaded();
        return ERROR;
        }

    LoadFlag loadFlag(m_tile->GetRootR());

    if (!m_tile->IsQueued())
        return SUCCESS; // this node was abandoned.

    if (SUCCESS == _ReadFromDb())
        {
        if (SUCCESS == LoadTile())
            {
            m_tile->SetIsReady();    // OK, we're all done loading and the other thread may now use this data. Set the "ready" flag.
            return SUCCESS;
            }
            
        // If we failed to load from the db, try from the source.
        }
        
    return _GetFromSource().then([=](BentleyStatus status)
        {
        if (status != SUCCESS)
            {
            if (m_loads != nullptr && m_loads->IsCanceled())
                m_tile->SetNotLoaded();     // Mark it as not loaded so we can retry again.
            else
                m_tile->SetNotFound();
            return ERROR;
            }

        if (SUCCESS != LoadTile())
            {
            m_tile->SetNotFound();
            return ERROR;
            }

        m_tile->SetIsReady();   // OK, we're all done loading and the other thread may now use this data. Set the "ready" flag.

        // On a successful load, store the tile in the cache.
        _SaveToDb();

        return SUCCESS;
        });
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
folly::Future<BentleyStatus> TileLoader::_GetFromSource()
    {
    bool isHttp = (0 == strncmp("http:", m_fileName.c_str(), 5) || 0 == strncmp("https:", m_fileName.c_str(), 6));

    if (isHttp)
        {
        HttpDataQuery query(m_fileName, m_loads);
        return query.Perform(m_tileBytes);
        }

    FileDataQuery query(m_fileName, m_loads);
    return query.Perform(m_tileBytes);
    }

/*---------------------------------------------------------------------------------**//**
* Attempt to load a node from the local cache.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileLoader::_ReadFromDb()
    {
    auto cache = m_tile->GetRootR().GetCache();
    if (!cache.IsValid())
        return ERROR;

    if (true)
        {
        RealityData::Cache::AccessLock lock(*cache);

        CachedStatementPtr stmt;
        if (BE_SQLITE_OK != cache->GetDb().GetCachedStatement(stmt, "SELECT Data,DataSize,ROWID FROM " TABLE_NAME_TileTree " WHERE Filename=?"))
            return ERROR;

        stmt->ClearBindings();
        stmt->BindText(1, m_cacheKey, Statement::MakeCopy::No);
        if (BE_SQLITE_ROW != stmt->Step())
            return ERROR;

        m_tileBytes.SaveData((Byte*) stmt->GetValueBlob(0), stmt->GetValueInt(1));
        m_tileBytes.SetPos(0);

        uint64_t rowId = stmt->GetValueInt64(2);
        if (BE_SQLITE_OK == cache->GetDb().GetCachedStatement(stmt, "UPDATE " TABLE_NAME_TileTree " SET Created=? WHERE ROWID=?"))
            {
            stmt->BindInt64(1, BeTimeUtilities::GetCurrentTimeAsUnixMillis());
            stmt->BindInt64(2, rowId);
            if (BE_SQLITE_DONE != stmt->Step())
                {
                BeAssert(false);
                }
            }
        }

    if (m_loads != nullptr)
        {
        m_loads->m_fromDb.IncrementAtomicPre(std::memory_order_relaxed);
        m_loads = nullptr;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Save the data for a tile into the tile cache. Note that this is also called for the non-tile files.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileLoader::_SaveToDb()
    {
    auto cache = m_tile->GetRootR().GetCache();
    if (!cache.IsValid())
        return ERROR;

    BeAssert(!m_cacheKey.empty());
    BeAssert(m_tileBytes.HasData());

    RealityData::Cache::AccessLock lock(*cache);

    // "INSERT OR REPLACE" so we can update old data that we failed to load.
    CachedStatementPtr stmt;
    auto rc = cache->GetDb().GetCachedStatement(stmt, "INSERT OR REPLACE INTO " TABLE_NAME_TileTree " (Filename,Data,DataSize,Created) VALUES (?,?,?,?)");

    BeAssert(rc == BE_SQLITE_OK);
    BeAssert(stmt.IsValid());

    stmt->ClearBindings();
    stmt->BindText(1, m_cacheKey, Statement::MakeCopy::No);
    stmt->BindBlob(2, m_tileBytes.GetData(), (int) m_tileBytes.GetSize(), Statement::MakeCopy::No);
    stmt->BindInt64(3, (int64_t) m_tileBytes.GetSize());

    if (m_tile.IsValid()) // for the root, store NULL for time. That way it will never get purged.
        stmt->BindInt64(4, BeTimeUtilities::GetCurrentTimeAsUnixMillis());

    rc = stmt->Step();
    if (BE_SQLITE_DONE != rc)
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
HttpDataQuery::HttpDataQuery(Utf8StringCR url, LoadStatePtr loads) : m_request(url), m_loads(loads), m_responseBody(Http::HttpByteStreamBody::Create())
    {
    m_request.SetResponseBody(m_responseBody);
    if (nullptr != loads)
        m_request.SetCancellationToken(loads);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
BentleyStatus HttpDataQuery::Perform(ByteStream& data)
    {
    m_response = GetRequest().Perform();

    if (Http::ConnectionStatus::OK != m_response.GetConnectionStatus() || Http::HttpStatus::OK != m_response.GetHttpStatus())
        return ERROR;

    data = std::move(m_responseBody->GetByteStream());

    if (m_loads != nullptr)
        {
        m_loads->m_fromHttp.IncrementAtomicPre(std::memory_order_relaxed);
        m_loads = nullptr; // for debugging, mostly
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileDataQuery::Perform(ByteStream& data)
    {
    BeFile dataFile;
    if (BeFileStatus::Success != dataFile.Open(m_fileName.c_str(), BeFileAccess::Read))
        return ERROR;

    if (BeFileStatus::Success != dataFile.ReadEntireFile(data))
        return ERROR;

    if (m_loads != nullptr)
        {
        m_loads->m_fromFile.IncrementAtomicPre(std::memory_order_relaxed);
        m_loads = nullptr;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Create the table to hold entries in this TileCache
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileCache::_Prepare() const 
    {
    if (m_db.TableExists(TABLE_NAME_TileTree))
        return SUCCESS;
        
    Utf8CP ddl = "Filename CHAR PRIMARY KEY,Data BLOB,DataSize BIGINT,Created BIGINT";
    if (BE_SQLITE_OK == m_db.CreateTable(TABLE_NAME_TileTree, ddl))
        return SUCCESS;

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
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

    uint64_t garbageSize = sum - (m_allowedSize * .95); // 5% slack to avoid purging often

    CachedStatementPtr selectStatement;
    m_db.GetCachedStatement(selectStatement, "SELECT DataSize,Created FROM " TABLE_NAME_TileTree " ORDER BY Created ASC");

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

    CachedStatementPtr deleteStatement;
    m_db.GetCachedStatement(deleteStatement, "DELETE FROM " TABLE_NAME_TileTree " WHERE Created <= ?");
    deleteStatement->BindInt64(1, creationDate);

    return BE_SQLITE_DONE == deleteStatement->Step() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::ClearAllTiles()
    {
    if (!m_rootTile.IsValid())
        return;

    m_rootTile->SetAbandoned();
    m_rootTile = nullptr;
    WaitForAllLoads();

    m_cache = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Root::DeleteCacheFile()
    {
    ClearAllTiles();
    return BeFileNameStatus::Success == m_localCacheName.BeDeleteFile() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::CreateCache(Utf8CP realityCacheName, uint64_t maxSize)
    {
    if (!IsHttp()) 
        return;
        
    m_localCacheName = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName();
    m_localCacheName.AppendToPath(BeFileName(realityCacheName));
    m_localCacheName.AppendExtension(L"TileCache");

    m_cache = new TileCache(maxSize);
    if (SUCCESS != m_cache->OpenAndPrepare(m_localCacheName))
        m_cache = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> Root::_RequestTile(TileR tile, LoadStatePtr loads)
    {
    DgnDb::VerifyClientThread();

    if (!tile.IsNotLoaded()) // this should only be called when the tile is in the "not loaded" state.
        {
        BeAssert(false);
        return ERROR;
        }

    TileLoaderPtr loader = tile._CreateTileLoader(loads);
    if (!loader.IsValid())
        return ERROR;   

    if (loads)
        loads->m_requested.IncrementAtomicPre(std::memory_order_relaxed);

    tile.SetIsQueued(); // mark as queued so we don't request it again.

    return folly::via(&BeFolly::ThreadPool::GetIoPool(), [=]() {return loader->CreateTile();}); // add to download queue
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
Root::Root(DgnDbR db, TransformCR location, Utf8CP rootUrl, Render::SystemP system) : m_db(db), m_rootUrl(rootUrl), m_location(location), m_renderSystem(system)
    {
    m_isHttp = (0 == strncmp("http:", rootUrl, 5) || 0 == strncmp("https:", rootUrl, 6));

    if (m_isHttp)
        m_rootDir = m_rootUrl.substr(0, m_rootUrl.find_last_of("/"));
    else
        {
        BeFileName rootUrl(BeFileName::DevAndDir, BeFileName(m_rootUrl));
        BeFileName::FixPathName(rootUrl, rootUrl, false);
        m_rootDir = rootUrl.GetNameUtf8();
        }
    }

/*---------------------------------------------------------------------------------**//**
* This method gets called on the (valid) children of nodes as they are unloaded. Its purpose is to notify the loading
* threads that these nodes are no longer referenced and we shouldn't waste time loading them.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Tile::SetAbandoned() const
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
void Tile::_UnloadChildren(std::chrono::steady_clock::time_point olderThan) const
    {
    if (m_children.empty())
        return;

    if (m_childrenLastUsed > olderThan) // have we used this node's children recently?
        {
        // yes, this node has been used recently. We're going to keep it, but potentially unload its grandchildren
        for (auto const& child : m_children)
            child->_UnloadChildren(olderThan);

        return;
        }

    for (auto const& child : m_children)
        child->SetAbandoned();

    _OnChildrenUnloaded();
    m_children.clear();
    }

/*---------------------------------------------------------------------------------**//**
* ensure that this Tile's range includes its child's range.
* @bsimethod                                    Keith.Bentley                   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Tile::ExtendRange(DRange3dCR childRange) const
    {
    if (childRange.IsContained(m_range))
        return;

    m_range.Extend(childRange);

    if (nullptr != m_parent)
        m_parent->ExtendRange(childRange);
    }

/*---------------------------------------------------------------------------------**//**
* Draw this node. If it is too coarse, instead draw its children, if they are already loaded.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Tile::Draw(DrawArgsR args, int depth) const
    {
    DgnDb::VerifyClientThread();

    bool tooCoarse = true;

    if (IsDisplayable())    // some nodes are merely for structure and don't have any geometry
        {
        Frustum box(m_range);
        box = box.TransformBy(args.m_location);

        if (FrustumPlanes::Contained::Outside == args.m_context.GetFrustumPlanes().Contains(box))
            {
            if (_HasChildren())
                _UnloadChildren(args.m_purgeOlderThan);  // this node is completely outside the volume of the frustum. Unload any loaded children if they're expired.

            return;
            }

        double radius = args.GetTileRadius(*this); // use a sphere to test pixel size. We don't know the orientation of the image within the bounding box.
        DPoint3d center = args.GetTileCenter(*this);
        double pixelSize = radius / args.m_context.GetPixelSizeAtPoint(&center);
        tooCoarse = pixelSize > _GetMaximumSize();
        }

    auto children = _GetChildren(true); // returns nullptr if this node's children are not yet valid
    if (tooCoarse && nullptr != children)
        {
        // this node is too coarse for current view, don't draw it and instead draw its children
        m_childrenLastUsed = args.m_now; // save the fact that we've used our children to delay purging them if this node becomes unused

        for (auto const& child : *children)
            child->Draw(args, depth+1);

        return;
        }
    
    // This node is either fine enough for the current view or has some unloaded children. We'll draw it.
    _DrawGraphics(args, depth);

    if (!_HasChildren()) // this is a leaf node - we're done
        return;

    if (!tooCoarse)
        {
        // This node was fine enough for the current zoom scale and was successfully drawn. If it has loaded children from a previous pass, they're no longer needed.
        _UnloadChildren(args.m_purgeOlderThan);
        return;
        }

    // this node is too coarse (even though we already drew it) but has unloaded children. Put it into the list of "missing tiles" so we'll draw its children when they're loaded.
    // NOTE: add all missing tiles, even if they're already queued.
    if (!IsNotFound())
        args.m_missing.Insert(depth, this); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementAlignedBox3d Tile::ComputeRange() const
    {
    if (m_range.IsValid())
        return m_range;

    auto const* children = _GetChildren(true); // only returns fully loaded children
    if (nullptr != children)
        {
        for (auto const& child : *children)
            m_range.UnionOf(m_range, child->ComputeRange()); // this updates the range of the top level node
        }

    return m_range;
    }

/*-----------------------------------------------------------------------------------**//**
* Count the number of tiles for this tile and all of its children.
* for diagnostics only
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
int Tile::CountTiles() const
    {
    int count = 1;

    auto const* children = _GetChildren(false); // only returns fully loaded children
    if (nullptr != children)
        {
        for (auto const& child : *children)
            count += child->CountTiles();
        }

    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
LoadState::~LoadState()
    {
    DEBUG_PRINTF("Load: canceled=%d, request=%d, nFile=%d, nHttp=%d, nDb=%d", m_canceled.load(), m_requested.load(), m_fromFile.load() , m_fromHttp.load(), m_fromDb.load());
    }

/*---------------------------------------------------------------------------------**//**
* Add the Render::Graphics from all tiles that were found from this _Draw request to the context.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawArgs::DrawGraphics(ViewContextR context)
    {
    if (m_graphics.m_entries.empty() && m_substitutes.m_entries.empty())
        return;

    ViewFlags flags = context.GetViewFlags();
    flags.SetRenderMode(Render::RenderMode::SmoothShade);
    flags.m_textures = true;
    flags.m_visibleEdges = false;
    flags.m_shadows = false;
    flags.m_ignoreLighting = true;

    if (!m_graphics.m_entries.empty())
        {
        DEBUG_PRINTF("drawing %d Tiles", m_graphics.m_entries.size());
        m_graphics.SetViewFlags(flags);
        auto branch = m_context.CreateBranch(m_graphics, &m_location, m_clip);
        BeAssert(m_graphics.m_entries.empty()); // CreateBranch should have moved them
        m_context.OutputGraphic(*branch, nullptr);
        }

    // Substitute tiles are drawn behind "real" tiles so that when they arrive the better tiles overwrite the substitute ones.
    if (!m_substitutes.m_entries.empty())
        {
        DEBUG_PRINTF("drawing %d substitute Tiles", m_substitutes.m_entries.size());
        DPoint3d offset = {0.0, 0.0, -1};
        Transform moveBack = Transform::From(offset);
        Transform location = Transform::FromProduct(m_location, moveBack);

        m_substitutes.SetViewFlags(flags);
        auto branch = m_context.CreateBranch(m_substitutes, &location, m_clip);
        BeAssert(m_substitutes.m_entries.empty()); // CreateBranch should have moved them
        m_context.OutputGraphic(*branch, nullptr);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Add all missing tiles that are in the "not loaded" state to the download queue.
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawArgs::RequestMissingTiles(RootR root, LoadStatePtr loads)
    {
    // This requests tiles in depth first order (the key for m_missing is the tile's depth). Could also include distance to frontplane sort too.
    for (auto const& tile : m_missing)
        {
        if (tile.second->IsNotLoaded())
            root._RequestTile(const_cast<TileR>(*tile.second), loads);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::ChildTiles const* QuadTree::Tile::_GetChildren(bool create) const
    {
    if (!_HasChildren()) // is this is the highest resolution tile?
        return nullptr;

    if (create && m_children.empty())
        {
        // this Tile has children, but we haven't created them yet. Do so now
        uint8_t level = m_id.m_level+1;
        uint32_t col = m_id.m_column*2;
        uint32_t row = m_id.m_row*2;
        for (int i=0; i<2; ++i)
            {
            for (int j=0; j<2; ++j)
                {
                auto child = _CreateChild(TileId(level, col+i, row+j));
                if (child.IsValid())
                    m_children.push_back(child);
                }
            }
        }

    return &m_children;
    }

/*---------------------------------------------------------------------------------**//**
* we do not have any graphics for this tile, try its (lower resolution) parent, recursively.
* @bsimethod                                    Keith.Bentley                   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool QuadTree::Tile::TryLowerRes(DrawArgsR args, int depth) const
    {
    Tile* parent = (Tile*) m_parent;
    if (depth <= 0 || nullptr == parent)
        {
        // DEBUG_PRINTF("no lower res");
        return false;
        }

    if (parent->HasGraphics())
        {
        //DEBUG_PRINTF("using lower res %d", depth);
        args.m_substitutes.Add(*parent->m_graphic);
        return true;
        }

    return parent->TryLowerRes(args, depth-1); // recursion
    }

/*---------------------------------------------------------------------------------**//**
* We do not have any graphics for this tile, try its immediate children. Not recursive.
* @bsimethod                                    Keith.Bentley                   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void QuadTree::Tile::TryHigherRes(DrawArgsR args) const
    {
    for (auto const& child : m_children)
        {
        Tile* quadChild = (Tile*) child.get();

        if (quadChild->HasGraphics())
            {
            //DEBUG_PRINTF("using higher res");
            args.m_substitutes.Add(*quadChild->m_graphic);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void QuadTree::Tile::_DrawGraphics(DrawArgsR args, int depth) const
    {
    if (!IsReady())
        {
        if (!IsNotFound())
            args.m_missing.Insert(depth, this);

        if (!TryLowerRes(args, 10))
            TryHigherRes(args);

        return;
        }

    if (m_graphic.IsValid())
        args.m_graphics.Add(*m_graphic);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
QuadTree::Root::Root(DgnDbR db, TransformCR trans, Utf8CP rootUrl, Dgn::Render::SystemP system, uint8_t maxZoom, uint32_t maxSize, double transparency) 
    : T_Super::Root(db, trans, rootUrl, system), m_maxZoom(maxZoom), m_maxPixelSize(maxSize)
    {
    m_tileColor = ColorDef::White();
    if (0.0 != transparency)
        m_tileColor.SetAlpha((Byte) (255.* transparency));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void QuadTree::Root::DrawInView(RenderContextR context)
    {
    if (!GetRootTile().IsValid())
        {
        BeAssert(false);
        return;
        }

    auto now = std::chrono::steady_clock::now();
    DrawArgs args(context, GetLocation(), now, now-GetExpirationTime());
    Draw(args);
    DEBUG_PRINTF("SheetView draw %d graphics, %d total, %d missing ", args.m_graphics.m_entries.size(), GetRootTile()->CountTiles(), args.m_missing.size());

    args.DrawGraphics(context);

    // Do we still have missing tiles?
    if (!args.m_missing.empty())
        {
        // yes, request them and schedule a progressive task to draw them as they arrive.
        LoadStatePtr loads = std::make_shared<LoadState>();
        args.RequestMissingTiles(*this, loads);
        context.GetViewport()->ScheduleTerrainProgressiveTask(*new ProgressiveTask(*this, args.m_missing, loads));
        }
    }

/*---------------------------------------------------------------------------------**//**
* Called periodically (on a timer) on the main thread to check for arrival of missing tiles.
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressiveTask::Completion QuadTree::ProgressiveTask::_DoProgressive(ProgressiveContext& context, WantShow& wantShow)
    {
    auto now = std::chrono::steady_clock::now();
    DrawArgs args(context, m_root.GetLocation(), now, now-m_root.GetExpirationTime());

    DEBUG_PRINTF("Map progressive %d missing", m_missing.size());

    for (auto const& node: m_missing)
        {
        auto stat = node.second->GetLoadStatus();
        if (stat == Tile::LoadStatus::Ready)
            node.second->Draw(args, node.first);        // now ready, draw it (this potentially generates new missing nodes)
        else if (stat != Tile::LoadStatus::NotFound)
            args.m_missing.Insert(node.first, node.second);     // still not ready, put into new missing list
        }

    args.RequestMissingTiles(m_root, m_loads);
    args.DrawGraphics(context);  // the nodes that newly arrived are in the GraphicBranch in the DrawArgs. Add them to the context

    m_missing.swap(args.m_missing); // swap the list of missing tiles we were waiting for with those that are still missing.

    DEBUG_PRINTF("Map after progressive still %d missing", m_missing.size());
    if (m_missing.empty()) // when we have no missing tiles, the progressive task is done.
        {
        m_loads = nullptr; // for debugging
        context.GetViewport()->SetNeedsHeal(); // unfortunately the newly drawn tiles may be obscured by lower resolution ones
        return Completion::Finished;
        }

    if (now > m_nextShow)
        {
        m_nextShow = now + std::chrono::seconds(1); // once per second
        wantShow = WantShow::Yes;
        }

    return Completion::Aborted;
    }

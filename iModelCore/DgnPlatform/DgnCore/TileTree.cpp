/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/TileTree.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <BeHttp/HttpRequest.h>
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

//=======================================================================================
// This object is created to load a single tile asynchronously. 
// @bsiclass                                                    Keith.Bentley   08/16
//=======================================================================================
struct TileData 
{
protected:
    Root& m_root;               // save this separate from tile, since that can be null
    TilePtr m_tile;             // tile to be loaded.
    Utf8String m_fileName;      // full file or URL name
    Utf8String m_shortName;     // for loading or saving to tile cache
    mutable StreamBuffer m_tileBytes; // when available, bytes are saved here
    StreamBuffer* m_output;     // for "non tile" requests
    mutable TileLoadsPtr m_loads;

public:
    struct TileLoader
    {
        Root& m_root;
        TileLoader(Root& root) : m_root(root) {root.StartTileLoad();}
        ~TileLoader() {m_root.DoneTileLoad();}
    };

    TileData(Utf8StringCR filename, TileP tile, Root& root, StreamBuffer* output, TileLoadsPtr loads) : m_fileName(filename), m_root(root), m_tile(tile), m_output(output), m_loads(loads)
        {
        if (tile)
            m_shortName = tile->_GetTileName(); // Note: we must save this in the ctor, since it is not safe to call this on other threads.
        }

    BentleyStatus DoRead() const 
        {
        if (m_loads!=nullptr && m_loads->IsCanceled())
            {
            if (m_tile.IsValid()) 
                m_tile->SetNotLoaded();

            return ERROR;
            }

        if (m_tile.IsValid() && !m_tile->IsQueued())
            return SUCCESS; // this node was abandoned.

        TileLoader loadFlag(m_root);
        return m_root.IsHttp() ? LoadFromHttp() : ReadFromFile();
        }

    BentleyStatus ReadFromFile() const;
    BentleyStatus LoadFromHttp() const;
    BentleyStatus LoadFromDb() const;
    BentleyStatus SaveToDb() const;
};

DEFINE_REF_COUNTED_PTR(TileData)
DEFINE_REF_COUNTED_PTR(TileCache)

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileData::ReadFromFile() const
    {
    BeFile dataFile;
    if (BeFileStatus::Success != dataFile.Open(m_fileName.c_str(), BeFileAccess::Read))
        {
        m_tile->SetNotFound();
        return ERROR;
        }

    if (BeFileStatus::Success != dataFile.ReadEntireFile(m_tileBytes))
        {
        m_tile->SetNotFound();
        return ERROR;
        }

    if (m_output)
        {
        *m_output = m_tileBytes;
        return SUCCESS;
        }

    if (SUCCESS != m_tile->_LoadTile(m_tileBytes, m_root))
        {
        m_tile->SetNotFound();
        return ERROR;
        }

    if (m_loads!=nullptr)
        m_loads->m_fromFile.IncrementAtomicPre(std::memory_order_relaxed);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Attempt to load a node from the local cache.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileData::LoadFromDb() const
    {
    auto cache = m_root.GetCache();
    if (!cache.IsValid())
        return ERROR;

    if (true)
        {
        RealityData::Cache::AccessLock lock(*cache);

        CachedStatementPtr stmt;
        if (BE_SQLITE_OK != cache->GetDb().GetCachedStatement(stmt, "SELECT Data,DataSize,ROWID FROM " TABLE_NAME_TileTree " WHERE Filename=?"))
            return ERROR;

        Utf8StringCR name = m_shortName.empty() ? m_fileName : m_shortName;
        stmt->ClearBindings();
        stmt->BindText(1, name, Statement::MakeCopy::No);
        if (BE_SQLITE_ROW != stmt->Step())
            return ERROR;

        m_tileBytes.SaveData((Byte*)stmt->GetValueBlob(0), stmt->GetValueInt(1));
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

    if (nullptr != m_output) // is this is the scene file?
        {
        *m_output = std::move(m_tileBytes); // yes, just save its data. We're going to load it synchronously
        return SUCCESS;
        }

    if (m_loads!=nullptr)
        {
        m_loads->m_fromDb.IncrementAtomicPre(std::memory_order_relaxed);
        m_loads=nullptr; 
        }

    BeAssert(m_tile->IsQueued());
    return m_tile->_LoadTile(m_tileBytes, m_root);
    }

/*---------------------------------------------------------------------------------**//**
* Load a node from an http source. This method runs on an IOThreadPool thread and waits for the http request to 
* complete or timeout.
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileData::LoadFromHttp() const
    {
    if (SUCCESS == LoadFromDb())
        return SUCCESS; // node was available from local cache

    Http::HttpByteStreamBodyPtr responseBody = Http::HttpByteStreamBody::Create();
    Http::Request request(m_fileName);
    request.SetResponseBody(responseBody);
    if (nullptr != m_loads)
        request.SetCancellationToken(m_loads);

    Http::Response response = request.Perform();

    if (Http::ConnectionStatus::OK != response.GetConnectionStatus() || Http::HttpStatus::OK != response.GetHttpStatus())
        {
        if (response.GetConnectionStatus() == Http::ConnectionStatus::Canceled)
            {
            m_tile->SetNotLoaded();
            return ERROR;
            }

        DEBUG_PRINTF("Tile Not Found %s", m_shortName.c_str());
        m_tile->SetNotFound();
        return ERROR;
        }

    if (nullptr != m_output) // is this is the scene file?
        {
        *m_output = std::move(responseBody->GetByteStream());
        return SUCCESS;
        }

    m_tileBytes = std::move(responseBody->GetByteStream());

    if (m_tile->IsAbandoned())
        return ERROR;

    BeAssert(m_tile->IsQueued());
    if (SUCCESS != m_tile->_LoadTile(m_tileBytes, m_root))
        return ERROR;

    if (m_loads!=nullptr)
        {
        m_loads->m_fromHttp.IncrementAtomicPre(std::memory_order_relaxed);
        m_loads=nullptr; // for debugging, mostly
        }

    SaveToDb();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Save the data for a tile into the tile cache. Note that this is also called for the non-tile files.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileData::SaveToDb() const
    {
    auto cache = m_root.GetCache();
    if (!cache.IsValid())
        return ERROR;

    BeAssert(m_tileBytes.HasData());

    RealityData::Cache::AccessLock lock(*cache);

    Utf8StringCR name = m_shortName.empty() ? m_fileName : m_shortName;
    CachedStatementPtr stmt;
    auto rc = cache->GetDb().GetCachedStatement(stmt, "INSERT INTO " TABLE_NAME_TileTree " (Filename,Data,DataSize,Created) VALUES (?,?,?,?)");

    BeAssert(rc==BE_SQLITE_OK);
    BeAssert(stmt.IsValid());

    stmt->ClearBindings();
    stmt->BindText(1, name, Statement::MakeCopy::No);
    stmt->BindBlob(2, m_tileBytes.GetData(), (int)m_tileBytes.GetSize(), Statement::MakeCopy::No);
    stmt->BindInt64(3, (int64_t)m_tileBytes.GetSize());

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

    BeAssert (runningSum >= garbageSize);
    uint64_t creationDate = selectStatement->GetValueInt64(1);
    BeAssert (creationDate > 0);

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
void Root::CreateCache(uint64_t maxSize)
    {
    if (!IsHttp()) 
        return;

    m_cache = new TileCache(maxSize);
    if (SUCCESS != m_cache->OpenAndPrepare(m_localCacheName))
        m_cache = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> Root::_RequestFile(Utf8StringCR fileName, StreamBuffer& buffer)
    {
    DgnDb::VerifyClientThread();

    TileData data(fileName, nullptr, *this, &buffer, nullptr);
    return folly::via(&BeFolly::IOThreadPool::GetPool(), [=](){return data.DoRead();});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> Root::_RequestTile(TileCR tile, TileLoadsPtr loads)
    {
    DgnDb::VerifyClientThread();

    if (!tile.IsNotLoaded()) // this should only be called when the tile is in the "not loaded" state.
        {
        BeAssert(false);
        return ERROR;
        }

    if (loads)
        loads->m_requested.IncrementAtomicPre(std::memory_order_relaxed);

    tile.SetIsQueued(); // mark as queued so we don't request it again.
    TileData data(_ConstructTileName(tile), (TileP) &tile, *this, nullptr, loads);
    return folly::via(&BeFolly::IOThreadPool::GetPool(), [=](){return data.DoRead();}); // add to download queue
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
Root::Root(DgnDbR db, TransformCR location, Utf8CP realityCacheName, Utf8CP rootUrl, Render::SystemP system) : m_db(db), m_rootUrl(rootUrl), m_location(location), m_renderSystem(system)
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

    m_localCacheName = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName();
    m_localCacheName.AppendToPath(BeFileName(realityCacheName));
    m_localCacheName.AppendExtension(L"TileCache");
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
    m_loadState.store(LoadState::Abandoned);
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

    BeAssert(m_parent==nullptr || !m_parent->m_range.IsValid() || m_range.IsContained(m_parent->m_range));

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
TileLoads::~TileLoads()
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
void DrawArgs::RequestMissingTiles(RootR root, TileLoadsPtr loads)
    {
    // This requests tiles in depth first order (the key for m_missing is the tile's depth). Could also include distance to frontplane sort too.
    for (auto const& tile : m_missing)
        {
        if (tile.second->IsNotLoaded())
            root._RequestTile(*tile.second, loads);
        }
    }

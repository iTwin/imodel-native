/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/TileTree.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <folly/BeFolly.h>
#include <BeHttp/HttpClient.h>

USING_NAMESPACE_TILETREE

#define TABLE_NAME_TileTree "TileTree2"   // Added 'ContentType' and 'Expires'.

BEGIN_UNNAMED_NAMESPACE

//=======================================================================================
// Manage the creation and cleanup of the local TileCache used by TileData
// @bsiclass                                                    Keith.Bentley   08/16
//=======================================================================================
struct TileCache : RealityData::Cache
{
    uint64_t m_allowedSize;
    BentleyStatus _Prepare() const override;
    BentleyStatus _Cleanup() const override;
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
    if (IsCanceledOrAbandoned())
        return ERROR;

    BeAssert(m_tile->IsQueued());
    return _LoadTile();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
folly::Future<BentleyStatus> TileLoader::Perform()
    {
    if (m_loads)
        m_loads->m_requested.IncrementAtomicPre(std::memory_order_relaxed);

    m_tile->SetIsQueued(); // mark as queued so we don't request it again.

    TileLoaderPtr me(this);
    auto loadFlag = std::make_shared<LoadFlag>(m_tile->GetRootR());  // Keep track of running requests so we can exit gracefully.

    return _ReadFromDb().then([me, loadFlag] (BentleyStatus status)
        {
        if (SUCCESS == status)
            return folly::makeFuture(SUCCESS);

        if (me->IsCanceledOrAbandoned())
            return folly::makeFuture(ERROR);

        return me->_GetFromSource();
        }).then(&BeFolly::ThreadPool::GetCpuPool(), [me, loadFlag] (BentleyStatus status)
            {
            DgnDb::SetThreadId(DgnDb::ThreadId::CpuPool); // for debugging

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
            me->_SaveToDb();    // don't wait on the save.
            return SUCCESS;
            });
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
folly::Future<BentleyStatus> TileLoader::_GetFromSource()
    {
    bool isHttp = (0 == strncmp("http:", m_resourceName.c_str(), 5) || 0 == strncmp("https:", m_resourceName.c_str(), 6));

    if (isHttp)
        {
        auto query = std::make_shared<HttpDataQuery>(m_resourceName, m_loads);

        TileLoaderPtr me(this);
        return query->Perform().then([me, query] (Http::Response const& response)
            {
            if (Http::ConnectionStatus::OK != response.GetConnectionStatus() || Http::HttpStatus::OK != response.GetHttpStatus())
                return ERROR;

            me->m_tileBytes = std::move(query->m_responseBody->GetByteStream());
            me->m_contentType = response.GetHeaders().GetContentType();
            me->m_saveToCache = query->GetCacheContolExpirationDate(me->m_expirationDate, response);

            return SUCCESS;
            });
        }

    auto query = std::make_shared<FileDataQuery>(m_resourceName, m_loads);

    TileLoaderPtr me(this);
    return query->Perform().then([me, query](ByteStream const& data)
        {
        if (!data.HasData())
            return ERROR;

        me->m_tileBytes = std::move(data);
        me->m_contentType = "";     // unknown
        me->m_expirationDate = 0;   // unknown

        return SUCCESS;
        });
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  12/2016
//----------------------------------------------------------------------------------------
folly::Future<BentleyStatus> TileLoader::_ReadFromDb()
    {
    auto cache = m_tile->GetRootR().GetCache();
    if (!cache.IsValid())
        return ERROR;

    TileLoaderPtr me(this);

    return folly::via(&BeFolly::ThreadPool::GetIoPool(), [me] ()
        {
        if (me->IsCanceledOrAbandoned())
            return ERROR;

        return me->DoReadFromDb();
        });
    }

/*---------------------------------------------------------------------------------**//**
* Attempt to load a tile from the local cache.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileLoader::DoReadFromDb()
    {
    auto cache = m_tile->GetRootR().GetCache();
    if (!cache.IsValid())
        return ERROR;

    if (true)
        {
        RealityData::Cache::AccessLock lock(*cache); // block writes to cache Db while we're reading

        enum Column : int {Data=0,DataSize=1,ContentType=2,Expires=3,Rowid=4};
        CachedStatementPtr stmt;    
        if (BE_SQLITE_OK != cache->GetDb().GetCachedStatement(stmt, "SELECT Data,DataSize,ContentType,Expires,ROWID FROM " TABLE_NAME_TileTree " WHERE Filename=?"))
            {
            BeAssert(false);
            return ERROR;
            }

        stmt->ClearBindings();
        stmt->BindText(1, m_cacheKey, Statement::MakeCopy::No);
        if (BE_SQLITE_ROW != stmt->Step())
            return ERROR;

        m_tileBytes.SaveData((Byte*) stmt->GetValueBlob(Column::Data), stmt->GetValueInt(Column::DataSize));
        m_tileBytes.SetPos(0);
        m_contentType = stmt->GetValueText(Column::ContentType);
        m_expirationDate = stmt->GetValueInt64(Column::Expires);

        m_saveToCache = false;  // We just load the data from cache don't save it and update timestamp only.

        uint64_t rowId = stmt->GetValueInt64(Column::Rowid);
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
        m_loads->m_fromDb.IncrementAtomicPre(std::memory_order_relaxed);   // just for debugging, really
        m_loads = nullptr;
        }

    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  12/2016
//----------------------------------------------------------------------------------------
folly::Future<BentleyStatus> TileLoader::_SaveToDb()
    {
    if (!m_saveToCache)
        return SUCCESS;

    auto cache = m_tile->GetRootR().GetCache();
    if (!cache.IsValid())
        return ERROR;

    TileLoaderPtr me(this);

    return folly::via(&BeFolly::ThreadPool::GetIoPool(), [me] ()
        {
        return me->DoSaveToDb();
        });
    }

/*---------------------------------------------------------------------------------**//**
* Save the data for a tile into the tile cache. Note that this is also called for the non-tile files.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileLoader::DoSaveToDb()
    {
    if (!m_saveToCache)
        return SUCCESS;

    auto cache = m_tile->GetRootR().GetCache();
    if (!cache.IsValid())
        return ERROR;

    BeAssert(!m_cacheKey.empty());
    BeAssert(m_tileBytes.HasData());

    RealityData::Cache::AccessLock lock(*cache);

    // "INSERT OR REPLACE" so we can update old data that we failed to load.
    CachedStatementPtr stmt;
    auto rc = cache->GetDb().GetCachedStatement(stmt, "INSERT OR REPLACE INTO " TABLE_NAME_TileTree " (Filename,Data,DataSize,ContentType,Created,Expires) VALUES (?,?,?,?,?,?)");

    BeAssert(rc == BE_SQLITE_OK);
    BeAssert(stmt.IsValid());

    stmt->ClearBindings();
    stmt->BindText(1, m_cacheKey, Statement::MakeCopy::No);
    stmt->BindBlob(2, m_tileBytes.GetData(), (int) m_tileBytes.GetSize(), Statement::MakeCopy::No);
    stmt->BindInt64(3, (int64_t) m_tileBytes.GetSize());
    stmt->BindText(4, m_contentType, Statement::MakeCopy::No);
    stmt->BindInt64(5, BeTimeUtilities::GetCurrentTimeAsUnixMillis());
    stmt->BindInt64(6, m_expirationDate);

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
HttpDataQuery::HttpDataQuery(Utf8StringCR url, TileLoadStatePtr loads) : m_request(url), m_loads(loads), m_responseBody(Http::HttpByteStreamBody::Create())
    {
    m_request.SetResponseBody(m_responseBody);
    if (nullptr != loads)
        m_request.SetCancellationToken(loads);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
folly::Future<Http::Response> HttpDataQuery::Perform()
    {
    TileLoadStatePtr loads = m_loads;

    return GetRequest().Perform().then([loads] (Http::Response response)
        {
        if (Http::ConnectionStatus::OK == response.GetConnectionStatus() && Http::HttpStatus::OK == response.GetHttpStatus() &&
            loads != nullptr)
            {
            loads->m_fromHttp.IncrementAtomicPre(std::memory_order_relaxed);
            }

        return response;
        });
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
bool HttpDataQuery::GetCacheContolExpirationDate(uint64_t& expirationDate, Http::Response const& response)
    {
    expirationDate = 0;

    if (!response.IsSuccess())
        return false;

    Utf8String cacheControl = response.GetHeaders().GetCacheControl();
    size_t offset = 0;
    Utf8String directive;
    while ((offset = cacheControl.GetNextToken(directive, ",", offset)) != Utf8String::npos)
        {
        // Not parsed:
        // "private" : means that the cache is for a single user. This is what we have.
        // "s-maxage": max age for shared cache(aka proxies). We have a private single-user cache, not relevant.

        if (directive.StartsWith("no-cache") || directive.StartsWith("no-store"))
            {
            // We are not allowed to cache this response. It may contain sensitive information, requires usage tracking by the server...
            expirationDate = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
            return false;
            }

        if (directive.StartsWith("max-age="))
            {
            int maxAge = atoi(directive.c_str() + strlen("max-age="));

            expirationDate = BeTimeUtilities::GetCurrentTimeAsUnixMillis() + (maxAge * 1000);
            }
        }

    // if cache-control did not provide a max-age we must use 'Expires' directive if present.
    if (0 == expirationDate)
        {
        Utf8CP expiresStr = response.GetHeaders().GetValue("Expires");
        if (nullptr == expiresStr || SUCCESS != Http::HttpClient::HttpDateToUnixMillis(expirationDate, expiresStr))
            {
            // if we cannot find an expiration date we are still allowed to cache.
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ByteStream> FileDataQuery::Perform()
    {
    auto filename = m_fileName;
    TileLoadStatePtr loads = m_loads;
    return folly::via(&BeFolly::ThreadPool::GetIoPool(), [filename, loads] ()
        {
        ByteStream data;
        BeFile dataFile;
        if (BeFileStatus::Success != dataFile.Open(filename.c_str(), BeFileAccess::Read))
            return data;

        if (BeFileStatus::Success != dataFile.ReadEntireFile(data))
            return data;

        if (loads != nullptr)
            loads->m_fromFile.IncrementAtomicPre(std::memory_order_relaxed);

        return data;
        });
    }

/*---------------------------------------------------------------------------------**//**
* Create the table to hold entries in this TileCache
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileCache::_Prepare() const
    {
    if (m_db.TableExists(TABLE_NAME_TileTree))
        return SUCCESS;

    return BE_SQLITE_OK == m_db.CreateTable(TABLE_NAME_TileTree, "Filename CHAR PRIMARY KEY,Data BLOB,DataSize BIGINT,ContentType TEXT,Created BIGINT,Expires BIGINT") ? SUCCESS : ERROR;
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
folly::Future<BentleyStatus> Root::_RequestTile(TileR tile, TileLoadStatePtr loads, Render::SystemP renderSys)
    {
    if (!tile.IsNotLoaded()) // this should only be called when the tile is in the "not loaded" state.
        {
        BeAssert(false);
        return ERROR;
        }

    TileLoaderPtr loader = tile._CreateTileLoader(loads, renderSys);
    if (!loader.IsValid())
        return ERROR;

    return loader->Perform();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
Root::Root(DgnDbR db, TransformCR location, Utf8CP rootResource, Render::SystemP system) : m_db(db), m_location(location), m_renderSystem(system)
    {
    // unless a root directory is specified, we assume it's http.
    m_isHttp = true;

    if (nullptr == rootResource)
        return;

    m_isHttp = (0 == strncmp("http:", rootResource, 5) || 0 == strncmp("https:", rootResource, 6));

    m_rootResource.assign (rootResource);

    if (m_isHttp)
        {
        m_rootResource = m_rootResource.substr(0, m_rootResource.find_last_of("/"));
        }
    else if (!m_rootResource.empty())
        {
        BeFileName rootDirectory(BeFileName::DevAndDir, BeFileName(m_rootResource));
        BeFileName::FixPathName(rootDirectory, rootDirectory, false);
        m_rootResource = rootDirectory.GetNameUtf8();
        m_isHttp = false;
        }
    }

void Tile::_DrawGraphics(DrawArgsR args, int depth) const { _GetGraphics(args.m_graphics, depth); }

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
void Tile::_UnloadChildren(BeTimePoint olderThan) const
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
        box.Multiply(args.GetLocation());

        if (FrustumPlanes::Contained::Outside == args.m_context.GetFrustumPlanes().Contains(box) ||
            ((nullptr != args.m_clip) && (ClipPlaneContainment::ClipPlaneContainment_StronglyOutside == args.m_clip->ClassifyPointContainment(box.m_pts, 8))))
            {
            if (_HasChildren())
                _UnloadChildren(args.m_purgeOlderThan);  // this node is completely outside the volume of the frustum or clip. Unload any loaded children if they're expired.

            return;
            }

        double radius = args.GetTileRadius(*this); // use a sphere to test pixel size. We don't know the orientation of the image within the bounding box.
        DPoint3d center = args.GetTileCenter(*this);

        static double s_minPixelSize = 1.0E-7;
        double pixelSize = radius / std::max(s_minPixelSize, args.m_context.GetPixelSizeAtPoint(&center));
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
* @bsimethod                                    Keith.Bentley                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Tile::Pick(PickArgsR args, int depth) const
    {
    DgnDb::VerifyClientThread();

    if (args.m_context.WasAborted())
        return;

    bool tooCoarse = true;

    if (IsDisplayable())    // some nodes are merely for structure and don't have any geometry
        {
        Frustum box(m_range);
        box.Multiply(args.GetLocation());

        if (FrustumPlanes::Contained::Outside == args.m_context.GetFrustumPlanes().Contains(box) ||
            ((nullptr != args.m_clip) && (ClipPlaneContainment::ClipPlaneContainment_StronglyOutside == args.m_clip->ClassifyPointContainment(box.m_pts, 8))))
            {
            return;
            }

        double radius = args.GetTileRadius(*this); // use a sphere to test pixel size. We don't know the orientation of the image within the bounding box.
        DPoint3d center = args.GetTileCenter(*this);
        double pixelSize = radius / args.m_context.GetPixelSizeAtPoint(&center);
        tooCoarse = pixelSize > _GetMaximumSize();
        }

    auto* children = _GetChildren(true); // returns nullptr if this node's children are not yet valid
    if (tooCoarse && nullptr != children)
        {
        for (auto const& child : *children)
            child->Pick(args, depth+1);

        return;
        }

    _PickGraphics(args, depth);
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
TileLoadState::~TileLoadState()
    {
    DEBUG_PRINTF("Load: canceled=%d, request=%d, nFile=%d, nHttp=%d, nDb=%d", m_canceled.load(), m_requested.load(), m_fromFile.load() , m_fromHttp.load(), m_fromDb.load());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawArgs::DrawBranch(ViewFlags flags, Render::GraphicBranch& branch, double branchOffset, Utf8CP title)
    {
    if (branch.m_entries.empty())
        return;

    DPoint3d offset = {0.0, 0.0, m_root.m_biasDistance + branchOffset};
    Transform location = Transform::FromProduct(GetLocation(), Transform::From(offset));
    DEBUG_PRINTF("drawing %d %s Tiles", branch.m_entries.size(), title);
    branch.SetViewFlags(flags);
    auto drawBranch = m_context.CreateBranch(branch, &location, m_clip);
    BeAssert(branch.m_entries.empty()); // CreateBranch should have moved them
    m_context.OutputGraphic(*drawBranch, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* Add the Render::Graphics from all tiles that were found from this draw request to the context.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawArgs::DrawGraphics()
    {
    ViewFlags flags = m_root._GetDrawViewFlags(m_context);
    DrawBranch(flags, m_graphics.m_graphics, 0.0, "Main");
    DrawBranch(flags, m_graphics.m_hiResSubstitutes, m_root.m_hiResBiasDistance, "hiRes");
    DrawBranch(flags, m_graphics.m_loResSubstitutes, m_root.m_loResBiasDistance, "loRes");
    }

/*---------------------------------------------------------------------------------**//**
* Add all missing tiles that are in the "not loaded" state to the download queue.
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawArgs::RequestMissingTiles(RootR root, TileLoadStatePtr loads)
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
    if (m_isLeaf) // if this is a leaf, it has no children
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
* @bsimethod                                    Keith.Bentley                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
ViewFlags Root::_GetDrawViewFlags(RenderContextCR context) const 
    {
    ViewFlags flags = context.GetViewFlags();
    flags.SetRenderMode(Render::RenderMode::SmoothShade);
    flags.SetShowTextures(true);
    flags.SetShowVisibleEdges(false);
    flags.SetShowShadows(false);
    flags.SetShowCameraLights(false);
    flags.SetShowSourceLights(false);
    flags.SetShowSolarLight(false);
    return flags;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::DrawInView(RenderListContext& context, TransformCR location, ClipVectorCP clips)
    {
    if (!GetRootTile().IsValid())
        {
        BeAssert(false);
        return;
        }

    auto now = BeTimePoint::Now();
    DrawArgs args(context, location, *this, now, now-GetExpirationTime(), clips);
    for (;;)
        {
        m_rootTile->Draw(args, 0);
        DEBUG_PRINTF("%s: %d graphics, %d tiles, %d missing ", _GetName(), args.m_graphics.m_graphics.m_entries.size(), GetRootTile()->CountTiles(), args.m_missing.size());

        // Do we still have missing tiles?
        if (args.m_missing.empty())
            break; // no, just draw what we've got

        // yes, request them
        TileLoadStatePtr loads = std::make_shared<TileLoadState>();
        args.RequestMissingTiles(*this, loads);

        if (!context.GetUpdatePlan().GetQuitTime().IsInFuture()) // do we want to wait for them? This is really just for thumbnails
            {
            // no, schedule a progressive pass for when they arrive
            context.GetViewport()->ScheduleProgressiveTask(*_CreateProgressiveTask(args, loads));
            break;
            }

        BeDuration::FromMilliseconds(20).Sleep(); // we want to wait. Give tiles some time to arrive
        args.Clear(); // clear graphics/missing from previous attempt
        }

    args.DrawGraphics();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::Pick(PickContext& context, TransformCR location, ClipVectorCP clips)
    {
    if (!GetRootTile().IsValid())
        return;

    PickArgs args(context, location, *this, clips);
    m_rootTile->Pick(args, 0);
    }

/*---------------------------------------------------------------------------------**//**
* we do not have any graphics for this tile, try its (lower resolution) parent, recursively.
* @bsimethod                                    Keith.Bentley                   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool QuadTree::Tile::TryLowerRes(DrawGraphicsR args, int depth) const
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
        args.m_loResSubstitutes.Add(*parent->m_graphic);
        return true;
        }

    return parent->TryLowerRes(args, depth-1); // recursion
    }

/*---------------------------------------------------------------------------------**//**
* We do not have any graphics for this tile, try its immediate children. Not recursive.
* @bsimethod                                    Keith.Bentley                   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void QuadTree::Tile::TryHigherRes(DrawGraphicsR args) const
    {
    for (auto const& child : m_children)
        {
        Tile* quadChild = (Tile*) child.get();

        if (quadChild->HasGraphics())
            {
            //DEBUG_PRINTF("using higher res");
            args.m_hiResSubstitutes.Add(*quadChild->m_graphic);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void QuadTree::Tile::_DrawGraphics(DrawArgsR args, int depth) const 
    {
    _GetGraphics(args.m_graphics, depth);

    if (!IsReady() && !IsNotFound())
        args.m_missing.Insert(depth, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void QuadTree::Tile::_GetGraphics(DrawGraphicsR args, int depth) const
    {
    if (!IsReady())
        {
        TryLowerRes(args, 10);
        TryHigherRes(args);
        return;
        }

    if (m_graphic.IsValid())
        args.m_graphics.Add(*m_graphic);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
QuadTree::Root::Root(DgnDbR db, TransformCR trans, Utf8CP rootDirectory, Dgn::Render::SystemP system, uint8_t maxZoom, uint32_t maxSize, double transparency)
    : T_Super::Root(db, trans, rootDirectory, system), m_maxZoom(maxZoom), m_maxPixelSize(maxSize)
    {
    m_tileColor = ColorDef::White();
    if (0.0 != transparency)
        m_tileColor.SetAlpha((Byte) (255.* transparency));
    }

/*---------------------------------------------------------------------------------**//**
* Called periodically (on a timer) on the client thread to check for arrival of missing tiles.
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressiveTask::Completion QuadTree::ProgressiveTask::_DoProgressive(RenderListContext& context, WantShow& wantShow)
    {
    auto now = BeTimePoint::Now();
    DrawArgs args(context, m_root.GetLocation(), m_root, now, now-m_root.GetExpirationTime(), m_root.m_clip.get());

    DEBUG_PRINTF("%s progressive %d missing", m_name.c_str(), m_missing.size());

    for (auto const& node: m_missing)
        {
        auto stat = node.second->GetLoadStatus();
        if (stat == Tile::LoadStatus::Ready)
            node.second->Draw(args, node.first);        // now ready, draw it (this potentially generates new missing nodes)
        else if (stat != Tile::LoadStatus::NotFound)
            args.m_missing.Insert(node.first, node.second);     // still not ready, put into new missing list
        }

    args.RequestMissingTiles(m_root, m_loads);
    args.DrawGraphics();  // the nodes that newly arrived are in the GraphicBranch in the DrawArgs. Add them to the context

    m_missing.swap(args.m_missing); // swap the list of missing tiles we were waiting for with those that are still missing.

    DEBUG_PRINTF("%s after progressive still %d missing", m_name.c_str(), m_missing.size());
    if (m_missing.empty()) // when we have no missing tiles, the progressive task is done.
        {
        m_loads = nullptr; // for debugging
        context.GetViewport()->SetNeedsHeal(); // unfortunately the newly drawn tiles may be obscured by lower resolution ones
        return Completion::Finished;
        }

    if (now > m_nextShow)
        {
        m_nextShow = now + BeDuration::Seconds(1); // once per second
        wantShow = WantShow::Yes;
        }

    return Completion::Aborted;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressiveTaskPtr QuadTree::Root::_CreateProgressiveTask(DrawArgs& args, TileLoadStatePtr loads)
    {
    return new QuadTree::ProgressiveTask(*this, args.m_missing, loads);
    }

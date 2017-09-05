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
#include <numeric>

#define WIP_SCALABLE_MESH

USING_NAMESPACE_TILETREE

#define TABLE_NAME_TileTree "TileTree3" // Moved 'Created' to a separate table
#define TABLE_NAME_TileTreeCreateTime "TileTreeCreateTime"

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
    m_tile->SetIsQueued(); // mark as queued so we don't request it again.

    TileLoaderPtr me(this);
    auto loadFlag = std::make_shared<LoadFlag>(m_loads);  // Keep track of running requests so we can exit gracefully.

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

        enum Column : int {Data=0,DataSize=1,ContentType=2,Expires=3,Rowid=4,Created=5};
        CachedStatementPtr stmt;    
        constexpr Utf8CP selectSql = "SELECT Data,DataSize,ContentType,Expires," TABLE_NAME_TileTree ".ROWID as TileRowId,"
            "Created," TABLE_NAME_TileTreeCreateTime ".ROWID as CreatedRowId FROM " TABLE_NAME_TileTree
            " JOIN " TABLE_NAME_TileTreeCreateTime " ON TileRowId=CreatedRowId WHERE Filename=?";

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
        if (_IsExpired(createTime))
            {
            cache->GetDb().GetCachedStatement(stmt, "DELETE FROM " TABLE_NAME_TileTree " WHERE ROWID=?");
            stmt->BindInt64(1, rowId);
            if (BE_SQLITE_DONE != stmt->Step())
                {
                BeAssert(false);
                return ERROR;
                }

            cache->GetDb().GetCachedStatement(stmt, "DELETE FROM " TABLE_NAME_TileTreeCreateTime " WHERE ROWID=?");
            stmt->BindInt64(1, rowId);
            if (BE_SQLITE_DONE != stmt->Step())
                {
                BeAssert(false);
                }

            return ERROR;
            }
        if (0 == stmt->GetValueInt64(Column::DataSize))
            {
            m_tileBytes.clear();
            }
        else
            {
            if (ZIP_SUCCESS != m_snappyFrom.Init(cache->GetDb(), TABLE_NAME_TileTree, "Data", stmt->GetValueInt64(Column::Rowid)))
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
            }
        m_tileBytes.SetPos(0);
        m_contentType = stmt->GetValueText(Column::ContentType);
        m_expirationDate = stmt->GetValueInt64(Column::Expires);
        
        m_saveToCache = false;  // We just load the data from cache don't save it and update timestamp only.

        if (BE_SQLITE_OK == cache->GetDb().GetCachedStatement(stmt, "UPDATE " TABLE_NAME_TileTreeCreateTime " SET Created=? WHERE ROWID=?"))
            {
            stmt->BindInt64(1, BeTimeUtilities::GetCurrentTimeAsUnixMillis());
            stmt->BindInt64(2, rowId);
            if (BE_SQLITE_DONE != stmt->Step())
                {
                BeAssert(false);
                }
            }
        }

    // ###TODO: Why? if (m_loads != nullptr)
    // ###TODO: Why?     m_loads = nullptr;

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

    RealityData::Cache::AccessLock lock(*cache);

    // "INSERT OR REPLACE" so we can update old data that we failed to load.
    CachedStatementPtr stmt;
    auto rc = cache->GetDb().GetCachedStatement(stmt, "INSERT OR REPLACE INTO " TABLE_NAME_TileTree " (Filename,Data,DataSize,ContentType,Expires) VALUES (?,?,?,?,?)");

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
    stmt->BindText(4, m_contentType, Statement::MakeCopy::No);
    stmt->BindInt64(5, m_expirationDate);

    rc = stmt->Step();
    if (BE_SQLITE_DONE != rc)
        {
        BeAssert(false);
        return ERROR;
        }

    // Write the tile creation time into separate table so that when we update it on next use of this tile, sqlite doesn't have to copy the potentially-huge data column
    // We don't know if we did an INSERT or UPDATE above...
    rc = cache->GetDb().GetCachedStatement(stmt, "SELECT ROWID FROM " TABLE_NAME_TileTree " WHERE Filename=?");
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

    // Try update existing row...
    rc = cache->GetDb().GetCachedStatement(stmt, "UPDATE " TABLE_NAME_TileTreeCreateTime " SET Created=? WHERE ROWID=?");
    BeAssert(BE_SQLITE_OK == rc && stmt.IsValid());

    stmt->BindInt64(1, BeTimeUtilities::GetCurrentTimeAsUnixMillis());
    stmt->BindInt64(2, rowId);
    if (BE_SQLITE_ROW != stmt->Step())
        {
        // We must have done an INSERT on tile tree table...
        rc = cache->GetDb().GetCachedStatement(stmt, "INSERT INTO " TABLE_NAME_TileTreeCreateTime " (Created) VALUES (?)");
        BeAssert(BE_SQLITE_OK == rc && stmt.IsValid());

        stmt->BindInt64(1, BeTimeUtilities::GetCurrentTimeAsUnixMillis());
        rc = stmt->Step();
        if (BE_SQLITE_DONE != rc)
            {
            BeAssert(false);
            return ERROR;
            }
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
        {
        BeAssert(m_db.TableExists(TABLE_NAME_TileTreeCreateTime));
        return SUCCESS;
        }
        
    if (BE_SQLITE_OK != m_db.CreateTable(TABLE_NAME_TileTreeCreateTime, "Created BIGINT"))
        return ERROR;

    return BE_SQLITE_OK == m_db.CreateTable(TABLE_NAME_TileTree,
        "Filename CHAR PRIMARY KEY,Data BLOB,DataSize BIGINT,ContentType TEXT,Expires BIGINT") ? SUCCESS : ERROR;
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
    constexpr Utf8CP selectSql = "SELECT DataSize,Created," TABLE_NAME_TileTree ".ROWID as TileRowId," TABLE_NAME_TileTreeCreateTime ".ROWID as CreatedRowId "
        " FROM " TABLE_NAME_TileTree " JOIN " TABLE_NAME_TileTreeCreateTime " ON TileRowId=CreatedRowId ORDER BY Created ASC";

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

    CachedStatementPtr deleteStatement;
    constexpr Utf8CP deleteSql = "DELETE FROM " TABLE_NAME_TileTree " WHERE ROWID IN (SELECT ROWID FROM " TABLE_NAME_TileTreeCreateTime " WHERE Created <= ?)";
    m_db.GetCachedStatement(deleteStatement, deleteSql);
    BeAssert(deleteStatement.IsValid());

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
    WaitForAllLoads();

    m_rootTile = nullptr;
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
void Root::CreateCache(Utf8CP realityCacheName, uint64_t maxSize, bool httpOnly)
    {
    if (httpOnly && !IsHttp()) 
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

    if (nullptr == loads)
        loads = std::make_shared<TileLoadState>(tile);

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::StartTileLoad(TileLoadStatePtr state) const
    {
    BeAssert(nullptr != state);
    BeMutexHolder lock(m_cv.GetMutex());
    BeAssert(m_activeLoads.end() == m_activeLoads.find(state));
    m_activeLoads.insert(state);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::DoneTileLoad(TileLoadStatePtr state) const
    {
    BeAssert(nullptr != state);
    BeMutexHolder lock(m_cv.GetMutex());
    // NB: If the load was canceled by RequestTiles(), it no longer exists in m_activeLoads
    m_activeLoads.erase(state);
    m_cv.notify_all();
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
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Tile::UnloadChildren(BeTimePoint olderThan) const
    {
    // This node may have initially had children and subsequently determined that it should be a leaf instead
    // - unload its now-useless children unconditionally
    if (!_HasChildren())
        olderThan = BeTimePoint::Now();

    _UnloadChildren(olderThan);
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

#if !defined(NDEBUG)
static int s_forcedDepth = -1;   // change this to a non-negative value in debugger in order to freeze the LOD of all tile trees.
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::SelectParent Tile::_SelectTiles(bvector<TileCPtr>& selected, DrawArgsR args) const
    {
    // ###TODO_ELEMENT_TILE: It would be nice to be able to generate only the tiles we need for the current frustum.
    // However, if we don't generate parents before children, then when the viewing frustum changes we will have nothing to draw until more tiles load.
    // So for now we do similarly to 3mx: only process children after parent is ready.
    DgnDb::VerifyClientThread();

#if defined(TILETREE_SKIP_INTERMEDIATES)
    Visibility vis = GetVisibility(args);
    if (Visibility::OutsideFrustum == vis)
        {
        UnloadChildren(args.m_purgeOlderThan);
        return SelectParent::No;
        }

    bool tooCoarse = Visibility::TooCoarse == vis;
    if (!tooCoarse)
        {
        // We want to draw this tile. Can we?
        auto selectParent = SelectParent::No;
        bool substitutingChildren = false;
        if (IsReady())
            {
            if (_HasGraphics())
                selected.push_back(this);
            }
        else if (IsNotFound())
            {
            selectParent = SelectParent::Yes;
            }
        else
            {
            // We can't draw this tile. Request it, and choose something to draw in its place
            args.InsertMissing(*this);

            // If children are already loaded and ready to draw, draw them in place of this one
            auto children = _GetChildren(false);
            bool allChildrenReady = false;
            if (nullptr != children)
                {
                // We'll add visible children to the selected list while determining if all visible children are ready.
                // This way we only need to perform the frustum test once per child.
                size_t initialSize = selected.size();
                allChildrenReady = std::accumulate(children->begin(), children->end(), true, [&args, &selected](bool init, TilePtr const& arg)
                    {
                    if (!init || arg->IsContentCulled(args))
                        return init;
                    else if (!arg->IsReady())
                        return false;

                    if (arg->_HasGraphics())
                        selected.push_back(arg);

                    return true;
                    });

                // At least one child passed the frustum test, but it not ready. Remove any other children we added to the selected list.
                if (!allChildrenReady)
                    selected.resize(initialSize);
                }

            if (allChildrenReady)
                {
                m_childrenLastUsed = args.m_now;
                substitutingChildren = true;
                for (auto const& child : *children)
                    if (child->_HasGraphics() && !child->IsContentCulled(args))
                        selected.push_back(child);
                }
            else
                {
                selectParent = SelectParent::Yes;
                }
            }

        if (!substitutingChildren)
            UnloadChildren(args.m_purgeOlderThan);

        return selectParent;
        }

    // This node is too coarse. Try to select its children instead.
    auto children = _GetChildren(true);
    if (nullptr != children)
        {
        m_childrenLastUsed = args.m_now;
        bool drawParent = false;
        size_t initialSize = selected.size();
        for (auto const& child : *children)
            {
            if (SelectParent::Yes == child->_SelectTiles(selected, args))
                {
                drawParent = true;
                // NB: We must continue iterating children so that they can be requested if missing...
                }
            }

        // Selected children - we're done
        if (!drawParent)
            {
            m_childrenLastUsed = args.m_now;
            return SelectParent::No;
            }

        // Remove any tiles which were selected above - they will be replaced with this tile (or its parent)
        selected.resize(initialSize);
        }

    if (IsReady())
        {
        // We can draw this tile in place of its children
        if (_HasGraphics())
            selected.push_back(this);

        return SelectParent::No;
        }

    return SelectParent::Yes;
#else
    Visibility vis = GetVisibility(args);
    if (Visibility::OutsideFrustum == vis)
        {
        _UnloadChildren(args.m_purgeOlderThan);
        return SelectParent::No;
        }

    bool tooCoarse = Visibility::TooCoarse == vis;
    bool ready = IsReady();

    // ###TODO_ELEMENT_TILE: Revisit this post-YII. See below
    ///if (!ready)
    ///    args.InsertMissing(*this);

    // ###TODO_ELEMENT_TILE: Would like to be able to enqueue children before parent is ready - but also want to ensure parent is ready
    // before children. Otherwise when we e.g. zoom out, if parent is not ready we have nothing to draw.
    // The IsParentDisplayable() test below allows us to skip intermediate tiles, but never the first displayable tiles in the tree.
    bool skipThisTile = tooCoarse && (ready || IsParentDisplayable());
    auto children = skipThisTile ? _GetChildren(true) : nullptr;

    // ###TODO_ELEMENT_TILE: Revisit this post-YII. See above. Don't want to queue tile if we're skipping it!
    if (!ready && !skipThisTile)
        args.InsertMissing(*this);

    if (nullptr != children)
        {
        m_childrenLastUsed = args.m_now;
        bool drawParent = false;
        size_t initialSize = selected.size();

        for (auto const& child : *children)
            {
            if (SelectParent::Yes == child->_SelectTiles(selected, args))
                {
                drawParent = true;
                // NB: We must continue iterating children so that they can be requested if missing...
                }
            }

        if (!drawParent)
            {
            m_childrenLastUsed = args.m_now;
            return SelectParent::No;
            }

        selected.resize(initialSize);
        }

    if (!tooCoarse)
        _UnloadChildren(args.m_purgeOlderThan);

    if (ready)
        {
        if (_HasGraphics())
            selected.push_back(this);

        return SelectParent::No;
        }
    else if (_HasBackupGraphics())
        {
        // ###TODO_ELEMENT_TILE: Revisit post-YII. Caching previous graphics while regenerating tile to reduce flishy-flash when model changes.
        selected.push_back(this);
        return SelectParent::No;
        }

    return SelectParent::Yes;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool Tile::HasContentRange() const
    {
    auto const& contentRange = _GetContentRange();
    return &contentRange != &m_range && !contentRange.IsEqual(m_range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool Tile::IsCulled(ElementAlignedBox3d const& range, DrawArgsCR args) const
    {
    /// NO. if (!IsDisplayable())
    /// NO.     return false;

    // NOTE: frustum test is in world coordinates, tile clip is in tile coordinates
    Frustum box(range);
    bool isOutside = FrustumPlanes::Contained::Outside == args.m_context.GetFrustumPlanes().Contains(box.TransformBy(args.GetLocation()));
    bool clipped = !isOutside && (nullptr != args.m_clip) && (ClipPlaneContainment::ClipPlaneContainment_StronglyOutside == args.m_clip->ClassifyPointContainment(box.m_pts, 8));
    return isOutside || clipped;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::Visibility Tile::GetVisibility(DrawArgsCR args) const
    {
    // NB: We test for region culling before IsDisplayable() - otherwise we will never unload children of undisplayed tiles when
    // they are outside frustum
    if (IsRegionCulled(args))
        return Visibility::OutsideFrustum;

    // some nodes are merely for structure and don't have any geometry
    if (!IsDisplayable())
        return Visibility::TooCoarse;

#if !defined(NDEBUG)
    if (s_forcedDepth >= 0)
        return GetDepth() == s_forcedDepth ? Visibility::Visible : Visibility::TooCoarse;
#endif

    bool hasContentRange = HasContentRange();
    if (!_HasChildren())
        {
        if (hasContentRange && IsContentCulled(args))
            return Visibility::OutsideFrustum;

        return Visibility::Visible; // it's a leaf node
        }

    double radius = args.GetTileRadius(*this); // use a sphere to test pixel size. We don't know the orientation of the image within the bounding box.
    DPoint3d center = args.GetTileCenter(*this);

#if defined(LIMIT_MIN_PIXEL_SIZE)
    constexpr double s_minPixelSizeAtPoint = 1.0E-7;
    double pixelSize = radius / std::max(s_minPixelSizeAtPoint, args.m_context.GetPixelSizeAtPoint(&center));
#else
    double pixelSizeAtPt = args.m_context.GetPixelSizeAtPoint(&center);
    double pixelSize = 0.0 != pixelSizeAtPt ? radius / pixelSizeAtPt : 1.0E-3;
#endif

    if (pixelSize > _GetMaximumSize() * args.GetTileSizeModifier())
        return Visibility::TooCoarse;

    if (hasContentRange && IsContentCulled(args))
        return Visibility::OutsideFrustum;

    return Visibility::Visible;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::DrawInView(SceneContextR context)
    {
    if (!GetRootTile().IsValid())
        {
        BeAssert(false);
        return;
        }

    DrawArgs args = CreateDrawArgs(context);
    bvector<TileCPtr> selectedTiles = SelectTiles(args);

    std::sort(selectedTiles.begin(), selectedTiles.end(), [&](TileCPtr const& lhs, TileCPtr const& rhs)
        {
        return args.ComputeTileDistance(*lhs) < args.ComputeTileDistance(*rhs);
        });

    for (auto const& selectedTile : selectedTiles)
        {
        BeAssert(!selectedTile->IsRegionCulled(args));
        selectedTile->_DrawGraphics(args);
        }

#if defined(DEBUG_TILE_SELECTION)
    if (!selectedTiles.empty())
        THREADLOG.debugv("Selected %u tiles", static_cast<uint32_t>(selectedTiles.size()));
#endif

    args.DrawGraphics();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
DrawArgs Root::CreateDrawArgs(SceneContextR context)
    {
    auto now = BeTimePoint::Now();
    return DrawArgs(context, _GetTransform(context), *this, now, now-GetExpirationTime(), _GetClipVector());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<TileCPtr> Root::SelectTiles(SceneContextR context)
    {
    DrawArgs args = CreateDrawArgs(context);
    return SelectTiles(args);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<TileCPtr> Root::SelectTiles(DrawArgsR args)
    {
    bvector<TileCPtr> selectedTiles;
    if (!GetRootTile().IsValid())
        {
        BeAssert(false);
        return selectedTiles;
        }

    InvalidateDamagedTiles();

    GetRootTile()->_SelectTiles(selectedTiles, args);

    return selectedTiles;
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
    if (m_canceled.load())
        {
        DEBUG_PRINTF("Tile load canceled", m_canceled.load());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawArgs::DrawBranch(ViewFlagsOverridesCR flags, Render::GraphicBranch& branch)
    {
    if (branch.m_entries.empty())
        return;

    branch.SetViewFlagsOverrides(flags);
    auto drawBranch = m_context.CreateBranch(branch, m_context.GetDgnDb(), GetLocation(), m_clip);
    BeAssert(branch.m_entries.empty()); // CreateBranch should have moved them
    m_context.OutputGraphic(*drawBranch, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
ViewFlagsOverrides Root::_GetViewFlagsOverrides() const
    {
    ViewFlagsOverrides flags;

    flags.SetRenderMode(Render::RenderMode::SmoothShade);
    flags.SetShowVisibleEdges(false);                                                                                                                                                                                                                               
    flags.SetShowTextures(true);
    flags.SetShowShadows(false);
    flags.SetShowCameraLights(false);
    flags.SetShowSourceLights(false);
    flags.SetShowSolarLight(false);

    return flags;
    }
    
/*---------------------------------------------------------------------------------**//**
* Add the Render::Graphics from all tiles that were found from this _Draw request to the context.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawArgs::DrawGraphics()
    {
    // Allow the tile tree to specify how view flags should be overridden
    DrawBranch(m_root._GetViewFlagsOverrides(), m_graphics);
    }

/*---------------------------------------------------------------------------------**//**
* We want to draw these missing tiles, but they are not yet ready. They may already be
* queued for loading, or actively loading.
* If they are in the "not loaded" state, add them to the load queue.
* Any tiles which are currently loading/queued but are *not* in this missing set should
* be cancelled - we have determined we do not need them to draw the current frame.
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::RequestTiles(MissingNodesCR missingNodes)
    {
    uint32_t numCanceled = 0;

        {
        // First cancel any loading/queued tiles which are no longer needed
        BeMutexHolder lock(m_cv.GetMutex());
        for (auto iter = m_activeLoads.begin(); iter != m_activeLoads.end(); /**/)
            {
            auto& load = *iter;
            if (!load->IsCanceled() && !missingNodes.Contains(load->GetTile()))
                {
                ++numCanceled;
                load->SetCanceled();
                iter = m_activeLoads.erase(iter);
                }
            else
                {
                ++iter;
                }
            }
        }

    // This requests tiles ordered first by distance to camera, then by depth.
    for (auto const& missing : missingNodes)
        {
        if (missing.GetTile().IsNotLoaded())
            {
            TileLoadStatePtr loads = std::make_shared<TileLoadState>(missing.GetTile());
            _RequestTile(const_cast<TileR>(missing.GetTile()), loads, nullptr);
            }
        }

    //DEBUG_PRINTF("Missing %u Loading %u Canceled %u", static_cast<uint32_t>(missingNodes.size()), static_cast<uint32_t>(m_activeLoads.size()), numCanceled);
    UNUSED_VARIABLE(numCanceled);
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

    return m_children.empty() ? nullptr : &m_children;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void QuadTree::Tile::_DrawGraphics(DrawArgsR args) const
    {
    BeAssert(IsReady());
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
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
OctTree::Root::Root(DgnDbR db, TransformCR location, Utf8CP rootUrl, Render::SystemP system)
    : T_Super(db, location, rootUrl, system)
    {
    // 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::ChildTiles const* OctTree::Tile::_GetChildren(bool load) const
    {
    if (m_isLeaf)
        return nullptr;

    if (load && m_children.empty())
        {
        for (int i = 0; i < 2; i++)
            {
            for (int j = 0; j < 2; j++)
                {
                for (int k = 0; k < 2; k++)
                    {
                    auto child = _CreateChild(m_id.CreateChildId(i, j, k));
                    if (child.IsValid())
                        m_children.push_back(child);
                    }
                }
            }
        }

    return m_children.empty() ? nullptr : &m_children;
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
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void OctTree::Tile::_DrawGraphics(TileTree::DrawArgsR args) const
    {
    BeAssert(IsReady());
    BeAssert(_HasGraphics()); // _SelectTiles() checks this - does not select tiles with no graphics.
    if (_HasGraphics())
        args.m_graphics.Add(*m_graphic);
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
DRange3d OctTree::Tile::ComputeChildRange(OctTree::Tile& child) const
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
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
OctTree::TileId OctTree::TileId::GetRelativeId(OctTree::TileId parentId) const
    {
    BeAssert(parentId.m_level+1 == m_level);
    return TileId(parentId.m_level, m_i % 2, m_j % 2, m_k % 2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
OctTree::TileId OctTree::Tile::GetRelativeTileId() const
    {
    auto tileId = GetTileId();
    auto parent = GetOctParent();
    if (nullptr != parent)
        tileId = tileId.GetRelativeId(parent->GetTileId());

    return tileId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void MissingNodes::Insert(TileCR tile, double distance)
    {
    MissingNode toInsert(tile, distance);
    auto inserted = m_set.insert(toInsert);
    if (!inserted.second && distance < inserted.first->GetDistance())
        {
        // replace with closer distance
        m_set.erase(inserted.first);
        m_set.insert(toInsert);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool MissingNodes::Contains(TileCR tile) const
    {
    // ###TODO: Make this more efficient...
    return m_set.end() != std::find_if(m_set.begin(), m_set.end(), [&](MissingNodeCR arg) { return &arg.GetTile() == &tile; });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawArgs::InsertMissing(TileCR tile)
    {
    m_missing.Insert(tile, ComputeTileDistance(tile));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
double DrawArgs::ComputeTileDistance(TileCR tile) const
    {
    // Actually, distance squared...
    DPoint3d centroid = DPoint3d::FromInterpolate(tile.GetRange().low, 0.5, tile.GetRange().high);
    m_root.GetLocation().Multiply(centroid);
    return centroid.DistanceSquared(m_context.GetViewportR().GetCamera().GetEyePoint());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
double DrawArgs::GetTileSizeModifier() const
    {
    TargetCP target = m_context.GetViewportR().GetRenderTarget();
    return nullptr != target ? target->GetTileSizeModifier() : 1.0;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool StreamBuffer::ReadBytes(void* buf, uint32_t size)
    {
    ByteCP start = GetCurrent();

    if (nullptr == Advance(size)) 
        {
        BeAssert(false); 
        return false;
        }

    memcpy(buf, start, size);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
DirtyRanges DirtyRanges::Intersect(DRange3dCR range) const
    {
    auto end = std::partition(m_begin, m_end, [&range](DRange3dCR arg) { return arg.IntersectsWith(range); });
    return DirtyRanges(m_begin, end);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::MarkDamaged(DRange3dCR range)
    {
    Transform transformToTile;
    transformToTile.InverseOf(GetLocation());
    DRange3d tileRange;
    transformToTile.Multiply(tileRange, range);

    BeMutexHolder lock(m_cv.GetMutex());
    m_damagedRanges.push_back(tileRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::InvalidateDamagedTiles()
    {
    DgnDb::VerifyClientThread();
    BeAssert(m_rootTile.IsValid());

    BeMutexHolder lock(m_cv.GetMutex());
    if (m_damagedRanges.empty() || m_rootTile.IsNull())
        return;

    DirtyRanges dirty(m_damagedRanges);
    m_rootTile->Invalidate(dirty);

    m_damagedRanges.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Tile::Invalidate(DirtyRangesCR dirty)
    {
    DgnDb::VerifyClientThread();

    // NB: The caller has already filtered the ranges to include only those which intersect this tile's range.
    if (dirty.empty())
        return;

    // some nodes are solely for structured and contain no graphics, therefore do not need to be invalidated.
    if (IsDisplayable() && _IsInvalidated(dirty))
        {
        // This tile needs to be regenerated
        m_root.CancelTileLoad(*this);
        SetNotLoaded();
        _Invalidate();
        }

    // Test children. Note that we are only partitioning the subset of damaged ranges which intersect the parent.
    auto children = _GetChildren(false);
    if (nullptr == children)
        return;

    for (auto const& child : *children)
        {
        DirtyRanges childDirty = dirty.Intersect(child->GetRange());
        child->Invalidate(childDirty);
        }
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
    // ###TODO_ELEMENT_TILE: Bentley containers don't support 'transparent' comparators, meaning we can't compare a TileLoadStatePtr to a Tile even
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
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TileRequests::RequestMissing() const
    {
    for (auto const& kvp : m_map)
        if (!kvp.second.empty())
            kvp.first->RequestTiles(kvp.second);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
DrawArgs::DrawArgs(SceneContextR context, TransformCR location, RootR root, BeTimePoint now, BeTimePoint purgeOlderThan, ClipVectorCP clip)
    : TileArgs(location, root, clip), m_context(context), m_missing(context.m_requests.GetMissing(root)), m_now(now), m_purgeOlderThan(purgeOlderThan)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::QPoint3dList TriMeshTree::TriMesh::CreateParams::QuantizePoints() const
    {
    // ###TODO: Is the tile's range known yet, and do we expect the range of points within it to be significantly smaller?
    DRange3d range = DRange3d::NullRange();
    for (int32_t i = 0; i < m_numPoints; i++)
        range.Extend(DPoint3d::From(m_points[i]));

    Render::QPoint3dList qpts(range);
    qpts.reserve(m_numPoints);
    for (int32_t i = 0; i < m_numPoints; i++)
        qpts.Add(DPoint3d::From(m_points[i]));

    return qpts;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::OctEncodedNormalList TriMeshTree::TriMesh::CreateParams::QuantizeNormals() const
    {
    OctEncodedNormalList oens;
    if (nullptr != m_normals)
        {
        oens.resize(m_numPoints);
        for (size_t i = 0; i < m_numPoints; i++)
            {
            FPoint3d normal = m_normals[i];
            FVec3d vec = FVec3d::From(normal.x, normal.y, normal.z);
            oens[i] = OctEncodedNormal::From(vec);
            }
        }

    return oens;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::TriMeshArgs TriMeshTree::TriMesh::CreateTriMeshArgs(TextureP texture, FPoint2d const* textureUV) const
    {
    TriMeshArgs trimesh;
    trimesh.m_numIndices = m_indices.size();
    trimesh.m_vertIndex = (uint32_t const*) (m_indices.empty() ? nullptr : &m_indices.front());
    trimesh.m_numPoints = (uint32_t) m_points.size();
    trimesh.m_points  = m_points.empty() ? nullptr : &m_points.front();
    trimesh.m_normals = m_normals.empty() ? nullptr : &m_normals.front();
    trimesh.m_textureUV = textureUV;
    trimesh.m_pointParams = m_points.GetParams();
    trimesh.m_texture = texture;

    return trimesh;
    }

/*---------------------------------------------------------------------------------**//**
* Create a PolyfaceHeader from a Geometry
    * @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr TriMeshTree::TriMesh::GetPolyface() const
    {
    TriMeshArgs trimesh = CreateTriMeshArgs(nullptr, nullptr);
    return trimesh.ToPolyface();
    }

/*-----------------------------------------------------------------------------------**//**
* Construct a Geometry from a CreateParams and a Scene. The scene is necessary to get the Render::System, and this
* Geometry is only valid for that Render::System
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TriMeshTree::TriMesh::TriMesh(CreateParams const& args, RootR root, Dgn::Render::SystemP renderSys)
    {
    // After we create a Render::Graphic, we only need the points/indices/normals for picking.
    // To save memory, only store them if the model is locatable.
    if (root.IsPickable())
        {
        m_indices.resize(args.m_numIndices);
        memcpy(&m_indices.front(), args.m_vertIndex, args.m_numIndices * sizeof(int32_t));

        m_points = args.QuantizePoints();

        if (nullptr != args.m_normals)
            m_normals = args.QuantizeNormals();
        }

#if defined(WIP_SCALABLE_MESH) // texture may not be valid...
    if (nullptr == renderSys)
#else
    if (nullptr == renderSys || !args.m_texture.IsValid())
#endif
        return;

    auto trimesh = CreateTriMeshArgs(args.m_texture.get(), args.m_textureUV);

    m_graphics.push_back(renderSys->_CreateTriMesh(trimesh, root.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TriMeshTree::TriMesh::Draw(DrawArgsR args)
    {
    if (!m_graphics.empty())
        args.m_graphics.Add(m_graphics);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TriMeshTree::TriMesh::Pick(PickArgsR args)
    {
    if (m_indices.empty())
        return;

    auto graphic = args.m_context.CreateWorldGraphic(args.m_location);
    graphic->AddPolyface(*GetPolyface());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::SelectParent TriMeshTree::Tile::_SelectTiles(bvector<TileTree::TileCPtr>& selectedTiles, DrawArgsR args) const
    {
    Visibility vis = GetVisibility(args);
    if (Visibility::OutsideFrustum == vis)
        {
        _UnloadChildren(args.m_purgeOlderThan);
        return SelectParent::No;
        }

    bool tooCoarse = Visibility::TooCoarse == vis;
    auto children = _GetChildren(true);
#if defined(WIP_SCALABLE_MESH)
    if (tooCoarse && nullptr != children && !children->empty())
#else
    if (tooCoarse && nullptr != children)
#endif
        {
        m_childrenLastUsed = args.m_now;
        for (auto const& child : *children)
            {
            // 3mx requires that we load tiles recursively - we cannot jump directly to the tiles we actually want to draw...
            if (!child->IsReady())
                args.InsertMissing(*child);

            child->_SelectTiles(selectedTiles, args);
            }

        return SelectParent::No;
        }

    // This node is either fine enough for the current view or has some unloaded children. We'll select it.
    selectedTiles.push_back(this);

    if (!tooCoarse)
        {
        // This node was fine enough for the current zoom scale and was successfully drawn. If it has loaded children from a previous pass, they're no longer needed.
        _UnloadChildren(args.m_purgeOlderThan);
        }

    return SelectParent::No;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TriMeshTree::Tile::AddDebugRangeGraphics(DrawArgsR args) const
    {
    GraphicParams params;
    params.SetLineColor(ColorDef::Red());

    auto graphic = args.m_context.CreateWorldGraphic();
    graphic->ActivateGraphicParams(params);
    graphic->AddRangeBox(m_range);
    args.m_graphics.Add(*graphic->Finish());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TriMeshTree::Tile::_DrawGraphics(DrawArgsR args) const
    {
    if (_WantDebugRangeGraphics())
        AddDebugRangeGraphics(args);

    for (auto mesh : m_meshes)
        mesh->Draw(args);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TriMeshTree::Tile::_PickGraphics(PickArgsR args, int depth) const
    {
    for (auto mesh : m_meshes)
        mesh->Pick(args);
    }


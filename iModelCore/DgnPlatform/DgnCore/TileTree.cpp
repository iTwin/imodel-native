/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/TileTree.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <folly/BeFolly.h>
#include <BeHttp/HttpClient.h>
#include <numeric>

USING_NAMESPACE_TILETREE

// Obsolete versions of table storing tile data
#define TABLE_NAME_TileTree1 "TileTree"
#define TABLE_NAME_TileTree2 "TileTree2"

// 3rd version of this table: Moved 'Created' to a separate table to avoid sqlite potentially having to copy the
// big data blob when updating only the create time
#define TABLE_NAME_TileTree "TileTree3"

// Obsolete first version of this table: No primary key - expect rowid to match corresponding row in TileTree3.
// That failed because the tables could be updated in multiple threads simultaneously.
#define TABLE_NAME_TileTreeCreateTime1 "TileTreeCreateTime"

// Second version: Same primary key as tile data table.
#define TABLE_NAME_TileTreeCreateTime "TileTreeCreateTime2"

#define COLUMN_TileTree_FileName TABLE_NAME_TileTree ".FileName"
#define COLUMN_TileTreeCreateTime_FileName TABLE_NAME_TileTreeCreateTime ".FileName"
#define JOIN_TileTreeTables TABLE_NAME_TileTree " JOIN " TABLE_NAME_TileTreeCreateTime " ON " COLUMN_TileTree_FileName "=" COLUMN_TileTreeCreateTime_FileName

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

            T_HOST.GetTileAdmin()._OnNewTileReady(tile.GetRoot().GetDgnDb());
            tile.SetIsReady();   // OK, we're all done loading and the other thread may now use this data. Set the "ready" flag.
            
            // On a successful load, potentially store the tile in the cache.   
            if (nullptr == me->m_renderSys || me->m_renderSys->_DoCacheTiles())
                {
                auto saveToDb = me->_SaveToDb();

                // don't wait on the save unless caller wants to immediately extract data from cache.
                if (me->_WantWaitOnSave())
                    saveToDb.get();
                }

            return SUCCESS;
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileLoader::_WantWaitOnSave() const
    {
    return T_HOST.GetTileAdmin()._WantWaitOnSave(m_tile->GetRoot().GetDgnDb());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
folly::Future<BentleyStatus> TileLoader::_GetFromSource()
    {
    bool isHttp = (0 == strncmp("http:", m_resourceName.c_str(), 5) || 0 == strncmp("https:", m_resourceName.c_str(), 6));

    if (isHttp)
        {
        m_maxValidDuration = _GetMaxValidDuration();
        auto query = std::make_shared<HttpDataQuery>(m_resourceName, m_loads);

        TileLoaderPtr me(this);
        return query->Perform().then([me, query] (Http::Response const& response)
            {
            if (Http::ConnectionStatus::OK != response.GetConnectionStatus() || Http::HttpStatus::OK != response.GetHttpStatus())
                return ERROR;

            if (!query->m_responseBody->GetByteStream().HasData())
                return ERROR;

            me->m_tileBytes = std::move(query->m_responseBody->GetByteStream()); // NEEDSWORK this is a copy not a move...

            me->m_contentType = response.GetHeaders().GetContentType();
            me->m_saveToCache = query->GetCacheControlExpirationDate(me->m_expirationDate, me->m_maxValidDuration, response);

            return SUCCESS;
            });
        }                                           


    auto query = std::make_shared<FileDataQuery>(m_resourceName, m_loads);
 
    TileLoaderPtr me(this);
    return query->Perform().then([me, query](ByteStream const& data)
        {
        if (!data.HasData())
            return ERROR;

        me->m_tileBytes = std::move(data); // NEEDSWORK this is a copy not a move...
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
* @bsimethod                                                    Paul.Connelly   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileLoader::DropFromDb(RealityData::CacheR cache)
    {
    CachedStatementPtr stmt;
    cache.GetDb().GetCachedStatement(stmt, "DELETE FROM " TABLE_NAME_TileTree " WHERE FileName=?");
    stmt->BindText(1, m_cacheKey, Statement::MakeCopy::No);
    if (BE_SQLITE_DONE != stmt->Step())
        {
        BeAssert(false);
        return ERROR;
        }

    cache.GetDb().GetCachedStatement(stmt, "DELETE FROM " TABLE_NAME_TileTreeCreateTime " WHERE FileName=?");
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
    auto cache = m_tile->GetRootR().GetCache();
    if (!cache.IsValid())
        return ERROR;

    if (true)
        {
        RealityData::Cache::AccessLock lock(*cache); // block writes to cache Db while we're reading

        enum Column : int {Data,DataSize,ContentType,Expires,Created,Rowid};
        CachedStatementPtr stmt;
        constexpr Utf8CP selectSql = "SELECT Data,DataSize,ContentType,Expires,Created," TABLE_NAME_TileTree ".ROWID as TileRowId"
            " FROM " JOIN_TileTreeTables " WHERE " COLUMN_TileTree_FileName "=?";

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
            if (!_IsValidData())
                {
                m_tileBytes.clear();
                DropFromDb(*cache);
                return ERROR;
                }
            }

        m_tileBytes.SetPos(0);
        m_contentType = stmt->GetValueText(Column::ContentType);
        m_expirationDate = stmt->GetValueInt64(Column::Expires);
        
        if (!_IsCompleteData())
            return ERROR; // _GetFromSource() will be invoked to further process the cache data

        m_saveToCache = false;  // We just load the data from cache don't save it and update timestamp only.

        if (BE_SQLITE_OK == cache->GetDb().GetCachedStatement(stmt, "UPDATE " TABLE_NAME_TileTreeCreateTime " SET Created=? WHERE FileName=?"))
            {
            stmt->BindInt64(1, _GetCreateTime());
            stmt->BindText(2, m_cacheKey, Statement::MakeCopy::No);
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

        enum Column : int {Data,DataSize,ContentType,Expires,Created,Rowid};

        CachedStatementPtr stmt;
        constexpr Utf8CP selectSql = "SELECT Data,DataSize,ContentType,Expires,Created," TABLE_NAME_TileTree ".ROWID as TileRowId"
            " FROM " JOIN_TileTreeTables " WHERE " COLUMN_TileTree_FileName "=?";

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

    // Compress and save the data
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

    // Write the tile creation time into separate table so that when we update it on next use of this tile, sqlite doesn't have to copy the potentially-huge data column
    rc = cache->GetDb().GetCachedStatement(stmt, "INSERT OR REPLACE INTO " TABLE_NAME_TileTreeCreateTime " (FileName,Created) VALUES (?,?)");
    BeAssert(BE_SQLITE_OK == rc && stmt.IsValid());

    stmt->BindText(1, m_cacheKey, Statement::MakeCopy::No);
    stmt->BindInt64(2, _GetCreateTime());
    if (BE_SQLITE_DONE != stmt->Step())
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
    m_request.SetRetryOptions(Http::Request::RetryOption::ResetTransfer, 1);
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
bool HttpDataQuery::GetCacheControlExpirationDate(uint64_t& expirationDate, uint64_t maxValidDuration, Http::Response const& response)
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
       
    // We need to check to see whether the TileTree itself sets a maximum valid duration. For example, we enforce a 3-day expiration on Bing Map tiles.
    if (0 != maxValidDuration)
        {
        uint64_t maxAllowedExpirationDate = BeTimeUtilities::GetCurrentTimeAsUnixMillis() + maxValidDuration;

        // if we got no expiration date, or the furthestExpirationDate is sooner, we use that.
        if ( (0 == expirationDate) || (maxAllowedExpirationDate < expirationDate) )
            expirationDate = maxAllowedExpirationDate;
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
    // The 'create time' table was the most recently modified - 'tile data' table was not modified at that time.
    // If new 'create time' table exists, this db is up to date.
    if (m_db.TableExists(TABLE_NAME_TileTreeCreateTime))
        {
        BeAssert(m_db.TableExists(TABLE_NAME_TileTree));

        if (!_ValidateData())
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
    m_db.DropTableIfExists(TABLE_NAME_TileTree1);
    m_db.DropTableIfExists(TABLE_NAME_TileTree2);
    m_db.DropTableIfExists(TABLE_NAME_TileTreeCreateTime1);

    // Drop leftover 'tile data' table - otherwise the existing rows will lack corresponding 'create time' rows
    m_db.DropTableIfExists(TABLE_NAME_TileTree);

    // Create the tables
    return BE_SQLITE_OK == m_db.CreateTable(TABLE_NAME_TileTreeCreateTime, "Filename CHAR PRIMARY KEY,Created BIGINT")
        && BE_SQLITE_OK == m_db.CreateTable(TABLE_NAME_TileTree,
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
    constexpr Utf8CP deleteDataSql = "DELETE FROM " TABLE_NAME_TileTree " WHERE FileName IN"
        " (SELECT FileName FROM " TABLE_NAME_TileTreeCreateTime " WHERE Created <= ?)";
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
* @bsimethod                                                    Paul.Connelly   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName DgnPlatformLib::Host::TileAdmin::_GetRealityDataCacheFileName(Utf8CP realityCacheName) const
    {
    // TFS#784733: Navigator wants these in a subdirectory to simplify management of various other types of caches
    // NB: Only reality data caches go in that subdirectory - element tiles go next to the iModel
    BeFileName filename = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName();

    filename.AppendToPath(L"Tiles");
    filename.AppendSeparator();
    BeFileName::CreateNewDirectory(filename);

    filename.AppendToPath(WString(realityCacheName, true).c_str());
    filename.AppendExtension(L"TileCache");

    return filename;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName DgnPlatformLib::Host::TileAdmin::_GetElementCacheFileName(DgnDbCR db) const
    {
    // TFS#816125: Tile cache names must be unique. Easiest way to ensure that is to put in same directory as iModel.
    BeFileName filename = db.GetFileName();
    filename.AppendExtension(L"TileCache");
    return filename;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::CreateCache(Utf8CP realityCacheName, uint64_t maxSize, bool httpOnly)
    {
    if (httpOnly && !IsHttp())
        return;

    m_localCacheName = T_HOST.GetTileAdmin()._GetRealityDataCacheFileName(realityCacheName);
    m_cache = new TileCache(maxSize);
    if (SUCCESS != m_cache->OpenAndPrepare(m_localCacheName))
        m_cache = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> Root::_RequestTile(TileR tile, TileLoadStatePtr loads, Render::SystemP renderSys, BeDuration partialTimeout)
    {
    if (!tile.IsNotLoaded()) // this should only be called when the tile is in the "not loaded" state.
        {
        BeAssert(false);
        return ERROR;
        }

    if (nullptr == loads)
        loads = std::make_shared<TileLoadState>(tile, partialTimeout);

    TileLoaderPtr loader = tile._CreateTileLoader(loads, renderSys);
    if (!loader.IsValid())
        return ERROR;   
    
    return loader->Perform();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
Root::Root(GeometricModelCR model, TransformCR location, Utf8CP rootResource, Render::SystemP system)
    : Root(model.GetDgnDb(), model.GetModelId(), model.Is3d(), location, rootResource, system)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
Root::Root(DgnDbR db, DgnModelId modelId, TransformCR location, Utf8CP rootResource, Render::SystemP system)
    : Root(db, modelId, true, location, rootResource, system)
    {
    auto model = db.Models().GetModel(modelId);
//    BeAssert(model.IsValid());   This is null when called during conversion of ThreeMx.
    if (model.IsValid() && !model->Is3d())
        m_is3d = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
Root::Root(DgnDbR db, DgnModelId modelId, bool is3d, TransformCR location, Utf8CP rootResource, Render::SystemP system)
    : m_db(db), m_location(location), m_renderSystem(system), m_modelId(modelId), m_is3d(is3d)
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
    BeAssert(m_activeLoads.end() != m_activeLoads.find(state));
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

#if defined(DEBUG_UNLOAD_CHILDREN)
    BeDuration elapsed(BeTimePoint::Now() - olderThan);
    BeDuration sinceLastUsed(BeTimePoint::Now() - m_childrenLastUsed);
    THREADLOG.debugv("Unloading children after %f seconds (children last used %f seconds ago)", elapsed.ToSeconds(), sinceLastUsed.ToSeconds());
#endif

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
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::SelectParent Tile::_SelectTiles(bvector<TileCPtr>& selected, DrawArgsR args) const
    {
    DgnDb::VerifyClientThread();

    _ValidateChildren();

    if (IsNotFound())
        return SelectParent::Yes;

    Visibility vis = GetVisibility(args);
    if (Visibility::OutsideFrustum == vis)
        {
        _UnloadChildren(args.m_purgeOlderThan);
        return SelectParent::No;
        }

    bool tooCoarse = Visibility::TooCoarse == vis;
    bool ready = IsReady();

    // Would like to be able to enqueue children before parent is ready - but also want to ensure parent is ready
    // before children. Otherwise when we e.g. zoom out, if parent is not ready we have nothing to draw.
    // The IsParentDisplayable() test below allows us to skip intermediate tiles, but never the first displayable tiles in the tree.
    bool skipThisTile = tooCoarse && (ready || IsParentDisplayable());
    auto children = skipThisTile ? _GetChildren(true) : nullptr;

    // Don't enqueue a tile if we're skipping it...
    if (!ready && !skipThisTile)
        args.InsertMissing(*this);

    if (nullptr != children)
        {
        m_childrenLastUsed = args.m_now;
        bool drawParent = false;
        bool loadParent = false;
        size_t initialSize = selected.size();

        for (auto const& child : *children)
            {
            if (SelectParent::Yes == child->_SelectTiles(selected, args))
                {
                // NB: We must continue iterating children so that they can be requested if missing...
                drawParent = true;

                // This occurs with e.g. map tiles when we get back 'missing tile data' from the imagery provider.
                // Want to load the parent to draw in its place.
                // Note it's possible some children are not found and others are fine - load parent if any children are not found.
                if (child->IsNotFound())
                    loadParent = true;
                }
            }

        if (!drawParent)
            {
            m_childrenLastUsed = args.m_now;
            return SelectParent::No;
            }

        selected.resize(initialSize);

        if (loadParent && !ready)
            args.InsertMissing(*this);
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
        // Caching previous graphics while regenerating tile to reduce flishy-flash when model changes.
        selected.push_back(this);
        return SelectParent::No;
        }

    return SelectParent::Yes;
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
    Frustum worldBox = box.TransformBy(args.GetLocation());
    bool isOutside = FrustumPlanes::Contained::Outside == args.GetFrustumPlanes().Contains(worldBox);
    bool clipped = !isOutside && (nullptr != args.m_clip) && (ClipPlaneContainment::ClipPlaneContainment_StronglyOutside == args.m_clip->ClassifyPointContainment(box.m_pts, 8));
    return isOutside || clipped;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Tile::IsEmpty() const
    {
    // NB: A parent tile may be empty because the elements contained within it are all too small to contribute geometry -
    // children may not be empty.
    return IsReady() && !_HasGraphics() && !_HasChildren();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::Visibility Tile::GetVisibility(DrawArgsCR args) const
    {
    // NB: We test for region culling before IsDisplayable() - otherwise we will never unload children of undisplayed tiles when
    // they are outside frustum
    if (IsEmpty() || IsRegionCulled(args))
        return Visibility::OutsideFrustum;

    // some nodes are merely for structure and don't have any geometry
    if (!IsDisplayable())
        return Visibility::TooCoarse;

    bool hasContentRange = HasContentRange();
    if (!_HasChildren())
        {
        if (hasContentRange && IsContentCulled(args))
            return Visibility::OutsideFrustum;

        return Visibility::Visible; // it's a leaf node
        }

    if (GetDepth() < args.GetMinDepth())
        return Visibility::TooCoarse; // replace with children
    else if (GetDepth() == args.GetMaxDepth())
        return hasContentRange && IsContentCulled(args) ? Visibility::OutsideFrustum : Visibility::Visible;

    double radius = args.GetTileRadius(*this); // use a sphere to test pixel size. We don't know the orientation of the image within the bounding box.
    DPoint3d center = args.GetTileCenter(*this);

#if defined(LIMIT_MIN_PIXEL_SIZE)
    constexpr double s_minPixelSizeAtPoint = 1.0E-7;
    double pixelSize = radius / std::max(s_minPixelSizeAtPoint, args.GetPixelSizeAtPoint(&center));
#else
    double pixelSizeAtPt = args.GetPixelSizeAtPoint(&center);
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
    DrawArgs args = CreateDrawArgs(context);
    DrawInView(args);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::DrawInView(DrawArgsR args)
    {
    if (!GetRootTile().IsValid())
        {
        BeAssert(false);
        return;
        }

    bvector<TileCPtr> selectedTiles = SelectTiles(args);

    for (auto const& selectedTile : selectedTiles)
        selectedTile->_DrawGraphics(args);

#if defined(DEBUG_TILE_SELECTION)
    if (!selectedTiles.empty())
        THREADLOG.debugv("Selected %u tiles requested %u tiles", static_cast<uint32_t>(selectedTiles.size()), static_cast<uint32_t>(context.m_requests.GetMissing(*this).size()));
#endif

    args.DrawGraphics();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
DrawArgs Root::CreateDrawArgs(SceneContextR context)
    {
    auto now = BeTimePoint::Now();
    return DrawArgs(context, GetDisplayTransform(context), *this, now, now-GetExpirationTime(), _GetClipVector());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/17
+---------------+---------------+---------------+---------------+---------------+------*/
Transform Root::GetDisplayTransform(RenderContextR context) const
    {
    auto transform = _GetTransform(context);
    if (m_haveDisplayTransform)
        transform = Transform::FromProduct(m_displayTransform, transform);

    return transform;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::SetDisplayTransform(TransformCP tf)
    {
    m_haveDisplayTransform = nullptr != tf;
    if (m_haveDisplayTransform)
        m_displayTransform = *tf;
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

#if defined DEBUG_TILE_COUNTS
    THREADLOG.errorv("Selected %u tiles Missing %u", static_cast<uint32_t>(selectedTiles.size()), static_cast<uint32_t>(args.m_missing.size()));

    Tile::Counts counts;
    GetRootTile()->Count(counts);
    THREADLOG.errorv("Total %u Displayable %u Max Depth %u", static_cast<uint32_t>(counts.m_total), static_cast<uint32_t>(counts.m_displayable), counts.m_maxDepth);
#endif

    return selectedTiles;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Tile::Pick(PickArgsR args, uint32_t depth) const
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
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Tile::Count(Counts& counts) const
    {
    ++counts.m_total;
    if (GetDepth() > counts.m_maxDepth)
        counts.m_maxDepth = GetDepth();

    if (IsDisplayable())
        ++counts.m_displayable;

    auto children = _GetChildren(false);
    if (nullptr != children)
        for (auto const& child : *children)
            child->Count(counts);
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
    DrawBranch(m_viewFlagsOverrides, m_graphics);
    }

/*---------------------------------------------------------------------------------**//**
* We want to draw these missing tiles, but they are not yet ready. They may already be
* queued for loading, or actively loading.
* If they are in the "not loaded" state, add them to the load queue.
* Any tiles which are currently loading/queued but are *not* in this missing set should
* be cancelled - we have determined we do not need them to draw the current frame.
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::RequestTiles(MissingNodesCR missingNodes, BeDuration partialTimeout)
    {
    uint32_t numCanceled = 0;

        {
        // First cancel any loading/queued tiles which are no longer needed
        BeMutexHolder lock(m_cv.GetMutex());
        for (auto& load : m_activeLoads)
            {
            if (!load->IsCanceled() && !missingNodes.Contains(load->GetTile()))
                {
                load->SetCanceled();
                ++numCanceled;
                }
            }
        }

    for (auto const& missing : missingNodes)
        {
        if (missing->IsNotLoaded())
            {
            TileLoadStatePtr loads = std::make_shared<TileLoadState>(*missing, partialTimeout);
            _RequestTile(const_cast<TileR>(*missing), loads, nullptr, partialTimeout);
            }
        }

#if defined(DEBUG_TILE_REQUESTS)
    THREADLOG.warningv("Missing %u Loading %u Canceled %u", static_cast<uint32_t>(missingNodes.size()), static_cast<uint32_t>(m_activeLoads.size()), numCanceled);
#endif
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
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void QuadTree::Tile::_ValidateChildren() const
    {
    // This node may have initially had children and subsequently determined that it should be a leaf instead
    // - unload its now-useless children unconditionally
    if (m_isLeaf)
        _UnloadChildren(BeTimePoint::Now());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
QuadTree::Root::Root(GeometricModelCR model, TransformCR trans, Utf8CP rootUrl, Dgn::Render::SystemP system, uint8_t maxZoom, uint32_t maxSize, double transparency) 
    : T_Super::Root(model, trans, rootUrl, system), m_maxZoom(maxZoom), m_maxPixelSize(maxSize)
    {
    m_tileColor = ColorDef::White();
    if (0.0 != transparency)
        m_tileColor.SetAlpha((Byte) (255.* transparency));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
QuadTree::Root::Root(DgnDbR db, DgnModelId modelId, TransformCR trans, Utf8CP rootUrl, Dgn::Render::SystemP system, uint8_t maxZoom, uint32_t maxSize, double transparency) 
    : T_Super::Root(db, modelId, trans, rootUrl, system), m_maxZoom(maxZoom), m_maxPixelSize(maxSize)
    {
    m_tileColor = ColorDef::White();
    if (0.0 != transparency)
        m_tileColor.SetAlpha((Byte) (255.* transparency));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
OctTree::Root::Root(GeometricModelCR model, TransformCR location, Utf8CP rootUrl, Render::SystemP system)
    : T_Super(model, location, rootUrl, system)
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
        {
        args.m_graphics.Add(*m_graphic);
        if (_WantDebugRangeGraphics())
            AddDebugRangeGraphics(args);
        }
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
DRange3d OctTree::Tile::ComputeChildRange(OctTree::Tile& child, bool is2d) const
    {
    // Each dimension of the relative ID is 0 or 1, indicating which bisection of the range to take
    TileTree::OctTree::TileId relativeId = child.GetRelativeTileId();
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
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void OctTree::Tile::_ValidateChildren() const
    {
    // This node may have initially had children and subsequently determined that it should be a leaf instead
    // - unload its now-useless children unconditionally
    if (m_isLeaf)
        _UnloadChildren(BeTimePoint::Now());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void MissingNodes::Insert(TileCR tile, bool prioritize)
    {
    if (tile._IsPartial() || tile.IsNotLoaded())
        m_set.insert(Node(tile, prioritize));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool MissingNodes::Contains(TileCR tile) const
    {
    bool prioritize = !tile._IsPartial();
    Node toFind(tile, prioritize);
    if (m_set.end() != m_set.find(toFind))
        return true;

    // It's possible the tile was previously partial, then became complete on background thread.
    toFind.m_prioritize = !prioritize;
    return m_set.end() != m_set.find(toFind);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawArgs::InsertMissing(TileCR tile)
    {
    // Refine partial tiles with lower-priority than siblings with no graphics at all.
    m_missing.Insert(tile, !tile._IsPartial());
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
    double targetMod = nullptr != target ? target->GetTileSizeModifier() : 1.0;
    return targetMod * m_context.GetUpdatePlan().GetTileOptions().GetScale();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
BeTimePoint DrawArgs::GetDeadline() const
    {
    return m_context.GetUpdatePlan().GetTileOptions().GetDeadline();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t DrawArgs::GetMinDepth() const
    {
    return m_context.GetUpdatePlan().GetTileOptions().GetMinDepth();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t DrawArgs::GetMaxDepth() const
    {
// #define DEBUG_MAX_DEPTH 1
#if defined(DEBUG_MAX_DEPTH)
    return DEBUG_MAX_DEPTH;
#else
    return m_context.GetUpdatePlan().GetTileOptions().GetMaxDepth();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawArgs::InvalidateCopyrightInfo()
    {
    // Barry wants to update attribution info for bing map tiles based on the currently-selected set of tiles.
    auto& vp = m_context.GetViewportR();
    auto view = vp.GetSpatialViewControllerP();
    if (nullptr != view)
        {
        view->InvalidateCopyrightInfo();
        vp.InvalidateDecorations();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/18
+---------------+---------------+---------------+---------------+---------------+------*/
Render::FrustumPlanes const& DrawArgs::GetFrustumPlanes() const
    {
    // ###TODO: handle expanded frustum
    return m_context.GetFrustumPlanes();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/18
+---------------+---------------+---------------+---------------+---------------+------*/
double DrawArgs::GetPixelSizeAtPoint(DPoint3dCP origin) const
    {
    // ###TODO: handle expanded frustum
    return m_context.GetPixelSizeAtPoint(origin);
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
* @bsimethod                                                    Paul.Connelly   12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::SetIgnoreChanges(bool ignore) { m_ignoreChanges = ignore; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::OnAddToRangeIndex(DRange3dCR range, DgnElementId id)
    {
    if (m_ignoreChanges)
        return;

    MarkDamaged(range);
    _OnAddToRangeIndex(range, id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::OnRemoveFromRangeIndex(DRange3dCR range, DgnElementId id)
    {
    if (m_ignoreChanges)
        return;

    MarkDamaged(range);
    _OnRemoveFromRangeIndex(range, id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::OnUpdateRangeIndex(DRange3dCR oldRange, DRange3dCR newRange, DgnElementId id)
    {
    if (m_ignoreChanges)
        return;

    MarkDamaged(oldRange);
    MarkDamaged(newRange);
    _OnUpdateRangeIndex(oldRange, newRange, id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::InvalidateDamagedTiles()
    {
    // DgnDb::VerifyClientThread(); ###TODO: Relax this constraint when publishing view attachments to Cesium...
    BeAssert(m_rootTile.IsValid());

    BeMutexHolder lock(m_cv.GetMutex());
    if (m_damagedRanges.empty() || m_rootTile.IsNull())
        return;

    CancelAllTileLoads();
    while (m_activeLoads.size() > 0)
        m_cv.InfiniteWait(lock);

    DirtyRanges dirty(m_damagedRanges);
    UpdateRange(dirty);
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

    if (_IsInvalidated(dirty))
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
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::UpdateRange(DirtyRangesCR dirty)
    {
    TilePtr rootTile = GetRootTile();
    if (dirty.empty() || rootTile.IsNull())
        return;

    auto iter = dirty.begin();
    DRange3d range = *iter;
    ++iter;

    for (/*iter*/; iter != dirty.end(); ++iter)
        range.UnionOf(range, *iter);

    if (!range.IsContained(rootTile->GetRange()))
        rootTile->_UpdateRange(rootTile->GetRange(), range);
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
void TileRequests::RequestMissing(BeDuration partialTimeout) const
    {
#if defined(DEBUG_REQUEST_MISSING)
    size_t nRequested = 0;
    for (auto const& kvp : m_map)
        {
        for (auto const& missing : kvp.second)
            if (*missing.IsNotLoaded())
                ++nRequested;
        }

    if (0 < nRequested)
        THREADLOG.debugv("Requesting %llu tiles", nRequested);
#endif

    for (auto const& kvp : m_map)
        if (!kvp.second.empty())
            kvp.first->RequestTiles(kvp.second, partialTimeout);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileRequests::HasMissingTiles() const
    {
    for (auto const& kvp : m_map)
        if (!kvp.second.empty())
            return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
double TileArgs::GetTileRadius(TileCR tile) const
    {
    DRange3d range = tile.GetRange();
    m_location.Multiply(&range.low, 2);
    return 0.5 * (tile.GetRoot().Is3d() ? range.low.Distance(range.high) : range.low.DistanceXY(range.high));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
DrawArgs::DrawArgs(SceneContextR context, TransformCR location, RootR root, BeTimePoint now, BeTimePoint purgeOlderThan, ClipVectorCP clip)
    : TileArgs(location, root, clip), m_context(context), m_missing(context.m_requests.GetMissing(root)), m_now(now), m_purgeOlderThan(purgeOlderThan),
    m_viewFlagsOverrides(root._GetViewFlagsOverrides())
    {
    m_graphics.m_isBackgroundImagery = root._IsBackgroundImagery();
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

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void floatToDouble(double* pDouble, float const* pFloat, size_t n)
    {
    for (double* pEnd = pDouble + n; pDouble < pEnd; )
        *pDouble++ = *pFloat++;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  12/17
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr TriMeshTree::TriMesh::CreateParams::ToPolyface() const
    {
    PolyfaceHeaderPtr polyFace = PolyfaceHeader::CreateFixedBlockIndexed(3);

    BlockedVectorIntR pointIndex = polyFace->PointIndex();
    pointIndex.resize(m_numIndices);
    uint32_t const* pIndex = (uint32_t const*)m_vertIndex;
    uint32_t const* pEnd = pIndex + m_numIndices;
    int* pOut = &pointIndex.front();

    for (; pIndex < pEnd; )
        *pOut++ = 1 + *pIndex++;

    if (nullptr != m_points)
        {
        polyFace->Point().resize(m_numPoints);
        for (int i = 0; i < m_numPoints; i++)
            polyFace->Point()[i] = DVec3d::From(m_points[i].x, m_points[i].y, m_points[i].z);
        }

    if (nullptr != m_normals)
        {
        polyFace->Normal().resize(m_numPoints);
        for (int i = 0; i < m_numPoints; i++)
            polyFace->Normal()[i] = DVec3d::From(m_normals[i].x, m_normals[i].y, m_normals[i].z);
        }

    if (nullptr != m_textureUV)
        {
        polyFace->Param().resize(m_numPoints);
        floatToDouble(&polyFace->Param().front().x, &m_textureUV->x, 2 * m_numPoints);
        polyFace->ParamIndex() = pointIndex;
        }

    return polyFace;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TriMeshTree::TriMesh::CreateParams::FromTile(TextureCR tile, GraphicBuilder::TileCorners const& corners, FPoint3d* fpts, DgnDbR db)
    {
    for (int i = 0; i < 4; i++)
        {
        fpts[i] = FPoint3d::From(corners.m_pts[i].x, corners.m_pts[i].y, corners.m_pts[i].z);
        }
    m_numPoints  = 4;
    m_points     = fpts;
    m_normals    = nullptr;

    static int32_t indices[] = {0,1,2,2,1,3};
    m_numIndices = 6;
    m_vertIndex  = indices;

    static FPoint2d textUV[] = 
        {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {0.0f, 1.0f},
            {1.0f, 1.0f},
        };
    m_textureUV = textUV;
    m_texture = const_cast<Render::Texture*>(&tile);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Marc.Neely   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicPtr CreateTileGraphic(Render::GraphicR graphic, RootR root, TriMeshTree::TriMesh::CreateParams const& args)
    {
    Dgn::Render::SystemP renderSystem = root.GetRenderSystemP();
    if (nullptr == renderSystem || !root.GetModelId().IsValid())
        return &graphic;

    Feature feature(DgnElementId(root.GetModelId().GetValue()), DgnSubCategoryId(), DgnGeometryClass::Primary);
    FeatureTable features(root.GetModelId(), renderSystem->_GetMaxFeaturesPerBatch());
    features.GetIndex(feature);

    ElementAlignedBox3d range;
    range.InitFrom(args.m_points[0].x, args.m_points[0].y, args.m_points[0].z);
    for (size_t i = 1; i < args.m_numPoints; ++i)
        {
        range.Extend(args.m_points[i]);
        }

    return renderSystem->_CreateBatch(graphic, std::move(features), range);
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

    if (nullptr == renderSys)
        return;

    auto trimesh = CreateTriMeshArgs(args.m_texture.get(), args.m_textureUV);

    GraphicPtr graphic = renderSys->_CreateTriMesh(trimesh, root.GetDgnDb());

    m_graphics.push_back(CreateTileGraphic(*graphic, root, args));
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

    auto graphic = args.m_context.CreateSceneGraphic(args.m_location);
    graphic->AddPolyface(*GetPolyface());
    }

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  12/15
+===============+===============+===============+===============+===============+======*/
struct Clipper : PolyfaceQuery::IClipToPlaneSetOutput
{
    bool m_unclipped;
    bvector<PolyfaceHeaderPtr> m_output;

    Clipper() : m_unclipped(false) {}
    StatusInt _ProcessUnclippedPolyface(PolyfaceQueryCR) override {m_unclipped = true; return SUCCESS;}
    StatusInt _ProcessClippedPolyface(PolyfaceHeaderR mesh) override {PolyfaceHeaderPtr meshPtr = &mesh; m_output.push_back(meshPtr); return SUCCESS;}
    bvector<PolyfaceHeaderPtr>& ClipPolyface(PolyfaceQueryCR mesh, ClipVectorCR clip, bool triangulate) {clip.ClipPolyface(mesh, *this, triangulate); return m_output;}
    bool IsUnclipped() {return m_unclipped;}
}; // Clipper

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Mark.Schlosser  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void TriMeshTree::Root::ClipTriMesh(TriMeshList& triMeshList, TriMesh::CreateParams const& geomParams, Render::SystemP renderSys)
    {
    Clipper clipper;
    PolyfaceHeaderPtr polyface = geomParams.ToPolyface();
    bvector<PolyfaceHeaderPtr>& clippedPolyfaces = clipper.ClipPolyface(*polyface, *GetClipVector(), true);
    if (clipper.IsUnclipped())
        {
        triMeshList.push_front(new TriMesh(geomParams, *this, renderSys));
        }
    else
        {
        for (PolyfaceHeaderPtr clippedPolyface : clippedPolyfaces)
            {
            if (!clippedPolyface->IsTriangulated())
                clippedPolyface->Triangulate();

            if ((0 != clippedPolyface->GetParamCount() && clippedPolyface->GetParamCount() != clippedPolyface->GetPointCount()) || 
                (0 != clippedPolyface->GetNormalCount() && clippedPolyface->GetNormalCount() != clippedPolyface->GetPointCount()))
                clippedPolyface = PolyfaceHeader::CreateUnifiedIndexMesh(*clippedPolyface);

            size_t              numPoints = clippedPolyface->GetPointCount();
            bvector<int32_t>    indices;
            bvector<FPoint3d>   points(numPoints), normals(nullptr == clippedPolyface->GetNormalCP() ? 0 : numPoints);
            bvector<FPoint2d>   params(nullptr == clippedPolyface->GetParamCP() ? 0 : numPoints);

            for (size_t i=0; i<numPoints; i++)
                {
                points[i] = FPoint3d::From (clippedPolyface->GetPointCP()[i]);
                //bsiFPoint3d_initFromDPoint3d(&points[i], &clippedPolyface->GetPointCP()[i]);
                if (nullptr != clippedPolyface->GetNormalCP())
                    normals[i] = FPoint3d::From (clippedPolyface->GetNormalCP()[i]);
                    //bsiFPoint3d_initFromDPoint3d(&normals[i], &clippedPolyface->GetNormalCP()[i]);

                if (nullptr != clippedPolyface->GetParamCP())
                    {
                    params[i].x = clippedPolyface->GetParamCP()[i].x;
                    params[i].y = clippedPolyface->GetParamCP()[i].y;
                    }
                    //bsiFPoint2d_initFromDPoint2d(&params[i], &clippedPolyface->GetParamCP()[i]);
                }
            PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*clippedPolyface, true);
            for (visitor->Reset(); visitor->AdvanceToNextFace();)
                {   
                indices.push_back(visitor->GetClientPointIndexCP()[0]);
                indices.push_back(visitor->GetClientPointIndexCP()[1]);
                indices.push_back(visitor->GetClientPointIndexCP()[2]);
                }

            Dgn::TileTree::TriMeshTree::TriMesh::CreateParams clippedGeomParams;
            clippedGeomParams.m_numIndices = indices.size();
            clippedGeomParams.m_vertIndex  = &indices.front();
            clippedGeomParams.m_numPoints  = numPoints;
            clippedGeomParams.m_points     = &points.front();
            clippedGeomParams.m_normals    = normals.empty() ? nullptr : &normals.front();
            clippedGeomParams.m_textureUV  = params.empty() ? nullptr : &params.front();
            clippedGeomParams.m_texture    = geomParams.m_texture;

            triMeshList.push_front(new TriMesh(clippedGeomParams, *this, renderSys));
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mark.Schlosser   12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void TriMeshTree::Root::CreateGeometry(TriMeshList& triMeshList, TriMesh::CreateParams const& geomParams, Render::SystemP renderSys)
    {
    if (nullptr != GetClipVector())
        {
        ClipTriMesh(triMeshList, geomParams, renderSys);
        }
    else
        triMeshList.push_front(new TriMesh(geomParams, *this, renderSys));
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

    if (tooCoarse && nullptr != children)
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
        // Unloading a 3mx tile's children sets the parent as 'not ready', requiring us to reload the parent (which also loads the children again)
        // - so consider the children 'used' when the parent is selected
        m_childrenLastUsed = args.m_now;
        _UnloadChildren(args.m_purgeOlderThan);
        }

    return SelectParent::No;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Tile::AddDebugRangeGraphics(DrawArgsR args) const
    {
    GraphicParams params;
    params.SetLineColor(ColorDef::Red());

    auto graphic = args.GetContext().CreateSceneGraphic();
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
void TriMeshTree::Tile::_PickGraphics(PickArgsR args, uint32_t depth) const
    {
    for (auto mesh : m_meshes)
        mesh->Pick(args);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Marc.Neely   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicPtr Tile::CreateTileGraphic(Render::GraphicR graphic, DgnModelId modelId) const
    {
    Dgn::Render::SystemP renderSystem = m_root.GetRenderSystemP();
    if (nullptr == renderSystem || !modelId.IsValid())
        return &graphic;

    Feature feature(DgnElementId(modelId.GetValue()), DgnSubCategoryId(), DgnGeometryClass::Primary);
    FeatureTable features(modelId, renderSystem->_GetMaxFeaturesPerBatch());
    features.GetIndex(feature);

    return renderSystem->_CreateBatch(graphic, std::move(features), _GetContentRange());
    }

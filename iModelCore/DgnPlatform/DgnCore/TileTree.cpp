/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/TileTree.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <numeric>

//#define WIP_SCALABLE_MESH

USING_NAMESPACE_TILETREE

// Obsolete versions of table storing tile data
#define TABLE_NAME_TileTree3 "TileTree3"

// 4th version of this table: modified for iModelJs (imodel02 branch):
// Element tiles are now the only types of tiles produced and cached by the backend.
//  - Remove ContentType and Expires columns
//  - Add Metadata column, containing data formerly stored in the binary stream (geometry flags like 'is curved', zoom factor, etc)
#define TABLE_NAME_TileTree "TileTree4"

// Second version: Same primary key as tile data table.
#define TABLE_NAME_TileTreeCreateTime "TileTreeCreateTime2"

#define COLUMN_TileTree_TileId TABLE_NAME_TileTree ".TileId"
#define COLUMN_TileTreeCreateTime_TileId TABLE_NAME_TileTreeCreateTime ".TileId"
#define JOIN_TileTreeTables TABLE_NAME_TileTree " JOIN " TABLE_NAME_TileTreeCreateTime " ON " COLUMN_TileTree_TileId "=" COLUMN_TileTreeCreateTime_TileId

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
BentleyStatus TileLoader::Perform()
    {
    m_tile->SetIsQueued(); // mark as queued so we don't request it again.

    TileLoaderPtr me(this);
    auto loadFlag = std::make_shared<LoadFlag>(m_loads);  // Keep track of running requests so we can exit gracefully.

    auto status = _ReadFromDb();
    if (SUCCESS != status)
        status = me->IsCanceledOrAbandoned() ? ERROR : me->_GetFromSource();

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
    me->_SaveToDb();

    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  12/2016
//----------------------------------------------------------------------------------------
BentleyStatus TileLoader::_ReadFromDb()
    {
    auto cache = m_tile->GetRootR().GetCache();
    if (!cache.IsValid())
        return ERROR;

    TileLoaderPtr me(this);
    return me->IsCanceledOrAbandoned() ? ERROR : me->DoReadFromDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileLoader::DropFromDb(RealityData::CacheR cache)
    {
    CachedStatementPtr stmt;
    cache.GetDb().GetCachedStatement(stmt, "DELETE FROM " TABLE_NAME_TileTree " WHERE TileId=?");
    stmt->BindText(1, m_cacheKey, Statement::MakeCopy::No);
    if (BE_SQLITE_DONE != stmt->Step())
        {
        BeAssert(false);
        return ERROR;
        }

    cache.GetDb().GetCachedStatement(stmt, "DELETE FROM " TABLE_NAME_TileTreeCreateTime " WHERE TileId=?");
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

        enum Column : int {Data,DataSize,Metadata,Created,Rowid};
        CachedStatementPtr stmt;
        constexpr Utf8CP selectSql = "SELECT Data,DataSize,Metadata,Created," TABLE_NAME_TileTree ".ROWID as TileRowId"
            " FROM " JOIN_TileTreeTables " WHERE " COLUMN_TileTree_TileId "=?";

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
        Json::Value json;
        Json::Reader::Parse(stmt->GetValueText(Column::Metadata), json);
        m_tileMetadata.FromJson(json);
        
        if (!_IsCompleteData())
            return ERROR; // _GetFromSource() will be invoked to further process the cache data

        m_saveToCache = false;  // We just load the data from cache don't save it and update timestamp only.

        if (BE_SQLITE_OK == cache->GetDb().GetCachedStatement(stmt, "UPDATE " TABLE_NAME_TileTreeCreateTime " SET Created=? WHERE TileId=?"))
            {
            stmt->BindInt64(1, _GetCreateTime());
            stmt->BindText(2, m_cacheKey, Statement::MakeCopy::No);
            if (BE_SQLITE_DONE != stmt->Step())
                {
                BeAssert(false);
                }
            }
        }

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

        enum Column : int {Data,DataSize,Rowid};

        CachedStatementPtr stmt;
        constexpr Utf8CP selectSql = "SELECT Data,DataSize," TABLE_NAME_TileTree ".ROWID as TileRowId FROM " TABLE_NAME_TileTree " WHERE TileId=?";

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
BentleyStatus TileLoader::_SaveToDb()
    {
    if (!m_saveToCache)
        return SUCCESS;

    auto cache = m_tile->GetRootR().GetCache();
    if (!cache.IsValid())
        return ERROR;

    TileLoaderPtr me(this);
    return me->DoSaveToDb();
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
    auto rc = cache->GetDb().GetCachedStatement(stmt, "INSERT OR REPLACE INTO " TABLE_NAME_TileTree " (TileId,Data,DataSize,Metadata) VALUES (?,?,?,?)");

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

    Utf8String metadata = Json::FastWriter::ToString(m_tileMetadata.ToJson());
    stmt->BindText(4, metadata, Statement::MakeCopy::No);

    rc = stmt->Step();
    if (BE_SQLITE_DONE != rc)
        {
        BeAssert(false);
        return ERROR;
        }

    // Compress and save the data
    rc = cache->GetDb().GetCachedStatement(stmt, "SELECT ROWID FROM " TABLE_NAME_TileTree " WHERE TileId=?");
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
    rc = cache->GetDb().GetCachedStatement(stmt, "INSERT OR REPLACE INTO " TABLE_NAME_TileTreeCreateTime " (TileId,Created) VALUES (?,?)");
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

/*---------------------------------------------------------------------------------**//**
* Create the table to hold entries in this TileCache
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileCache::_Prepare() const 
    {
    // When current tables were TileTreeCreateTime2 and TileTree3, we changed the file extension from .TileCache to .Tiles
    // So we will never encounter an existing .Tiles file containing previous versions of those tables.
    if (m_db.TableExists(TABLE_NAME_TileTree))
        {
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
    m_db.DropTableIfExists(TABLE_NAME_TileTree3);

    // Create the tables
    if (!m_db.TableExists(TABLE_NAME_TileTreeCreateTime) && BE_SQLITE_OK != m_db.CreateTable(TABLE_NAME_TileTreeCreateTime, "TileId CHAR PRIMARY KEY,Created BIGINT"))
        return ERROR;

    return BE_SQLITE_OK == m_db.CreateTable(TABLE_NAME_TileTree,
        "TileId CHAR PRIMARY KEY,Data BLOB,DataSize BIGINT,Metadata TEXT") ? SUCCESS : ERROR;
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

    uint64_t garbageSize = sum - static_cast<uint64_t>(m_allowedSize * .95); // 5% slack to avoid purging often

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
    constexpr Utf8CP deleteDataSql = "DELETE FROM " TABLE_NAME_TileTree " WHERE TileId IN"
        " (SELECT TileId FROM " TABLE_NAME_TileTreeCreateTime " WHERE Created <= ?)";
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
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Root::RequestTile(TileR tile, TileLoadStatePtr loads)
    {
    if (!tile.IsNotLoaded()) // this should only be called when the tile is in the "not loaded" state.
        {
        BeAssert(false);
        return ERROR;
        }

    if (nullptr == loads)
        loads = std::make_shared<TileLoadState>(tile);

    TileLoaderPtr loader = tile._CreateTileLoader(loads);
    if (!loader.IsValid())
        return ERROR;   
    
    return loader->Perform();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
Root::Root(GeometricModelCR model, TransformCR location, Render::SystemR system)
    : Root(model.GetDgnDb(), model.GetModelId(), model.Is3d(), location, system)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
Root::Root(DgnDbR db, DgnModelId modelId, TransformCR location, Render::SystemR system)
    : Root(db, modelId, true, location, system)
    {
    auto model = db.Models().GetModel(modelId);
//    BeAssert(model.IsValid());   This is null when called during conversion of ThreeMx.
    if (model.IsValid() && !model->Is3d())
        m_is3d = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
Root::Root(DgnDbR db, DgnModelId modelId, bool is3d, TransformCR location, Render::SystemR system)
    : m_db(db), m_location(location), m_renderSystem(system), m_modelId(modelId), m_is3d(is3d)
    {
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
void Tile::UnloadChildren(BeTimePoint olderThan) const
    {
    if (m_children.empty())
        return;

    // ###TODO_IMODELCORE: Any use for the timestamp on backend? if (m_childrenLastUsed > olderThan) // have we used this node's children recently?
    if (false)
        {
        // yes, this node has been used recently. We're going to keep it, but potentially unload its grandchildren
        for (auto const& child : m_children)
            child->UnloadChildren(olderThan);

        return;
        }

    for (auto const& child : m_children)
        child->SetAbandoned();

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
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Tile::IsEmpty() const
    {
    // NB: A parent tile may be empty because the elements contained within it are all too small to contribute geometry -
    // children may not be empty.
    return IsReady() && !IsDisplayable() && !HasChildren();
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
* We want to draw these missing tiles, but they are not yet ready. They may already be
* queued for loading, or actively loading.
* If they are in the "not loaded" state, add them to the load queue.
* Any tiles which are currently loading/queued but are *not* in this missing set should
* be cancelled - we have determined we do not need them to draw the current frame.
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::RequestTiles(MissingNodesCR missingNodes)
    {
        {
        // First cancel any loading/queued tiles which are no longer needed
        BeMutexHolder lock(m_cv.GetMutex());
        for (auto& load : m_activeLoads)
            if (!load->IsCanceled() && !missingNodes.Contains(load->GetTile()))
                load->SetCanceled();
        }

    for (auto const& missing : missingNodes)
        {
        if (missing->IsNotLoaded())
            {
            TileLoadStatePtr loads = std::make_shared<TileLoadState>(*missing);
            RequestTile(const_cast<TileR>(*missing), loads);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::ChildTiles const* Tile::_GetChildren(bool load) const
    {
    if (IsLeaf())
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
DRange3d Tile::ComputeChildRange(Tile& child, bool is2d) const
    {
    // Each dimension of the relative ID is 0 or 1, indicating which bisection of the range to take
    TileId relativeId = child.GetRelativeTileId();
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
TileId TileId::GetRelativeId(TileId parentId) const
    {
    BeAssert(parentId.m_level+1 == m_level);
    return TileId(parentId.m_level, m_i % 2, m_j % 2, m_k % 2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileId Tile::GetRelativeTileId() const
    {
    auto tileId = GetTileId();
    auto parent = GetParent();
    if (nullptr != parent)
        tileId = tileId.GetRelativeId(parent->GetTileId());

    return tileId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void MissingNodes::Insert(TileCR tile, bool prioritize)
    {
    m_set.insert(Node(tile, prioritize));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool MissingNodes::Contains(TileCR tile) const
    {
    bool prioritize = false; // !tile._IsPartial();
    Node toFind(tile, prioritize);
    if (m_set.end() != m_set.find(toFind))
        return true;

    // It's possible the tile was previously partial, then became complete on background thread.
    toFind.m_prioritize = !prioritize;
    return m_set.end() != m_set.find(toFind);
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
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Render::SystemR Tile::GetRenderSystem() const { return m_root.GetRenderSystem(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value TileMetadata::ToJson() const
    {
    Json::Value json;
    json["zoomFactor"] = m_zoomFactor;
    json["flags"] = static_cast<uint32_t>(m_flags);
    m_contentRange.ToJson(json["contentRange"]);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
void TileMetadata::FromJson(Json::Value const& json)
    {
    m_zoomFactor = json["zoomFactor"].asDouble();
    m_flags = static_cast<TileFlags>(json["flags"].asUInt());
    m_contentRange.FromJson(json["contentRange"]);
    }


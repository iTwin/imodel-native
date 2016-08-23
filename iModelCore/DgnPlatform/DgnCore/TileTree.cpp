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
    uint64_t m_allowedSize = (1024*1024*1024); // 1 Gb
    virtual BentleyStatus _Prepare() const override;
    virtual BentleyStatus _Cleanup() const override;
};

//=======================================================================================
// This object is created to load a single tile asynchronously. 
// @bsiclass                                                    Keith.Bentley   08/16
//=======================================================================================
struct TileData
{
protected:
    Root& m_root;
    TilePtr m_tile;
    Utf8String m_fileName;
    Utf8String m_shortName;
    mutable StreamBuffer m_tileBytes;
    StreamBuffer* m_output;

public:
    TileData(Utf8StringCR filename, TileP tile, Root& root, StreamBuffer* output) : m_fileName(filename), m_root(root), m_tile(tile), m_output(output) 
        {
        if (tile)
            m_shortName = tile->_GetTileName(); // Note: we must save this in the ctor, since it is not safe to call this on other threads.
        }

    BentleyStatus DoRead() const 
        {
        if (m_tile.IsValid() && !m_tile->IsQueued())
            return SUCCESS; // this node was abandoned.

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

    if (SUCCESS != m_tile->_Read(m_tileBytes, m_root))
        {
        m_tile->SetNotFound();
        return ERROR;
        }

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
        CachedStatementPtr stmt;
        if (BE_SQLITE_OK != cache->GetDb().GetCachedStatement(stmt, "SELECT Data,DataSize FROM " TABLE_NAME_TileTree " WHERE Filename=?"))
            return ERROR;

        Utf8StringCR name = m_shortName.empty() ? m_fileName : m_shortName;
        stmt->ClearBindings();
        stmt->BindText(1, name, Statement::MakeCopy::No);
        if (BE_SQLITE_ROW != stmt->Step())
            return ERROR;

        m_tileBytes.SaveData((Byte*)stmt->GetValueBlob(0), stmt->GetValueInt(1));
        m_tileBytes.SetPos(0);
        }

    if (nullptr != m_output) // is this is the scene file?
        {
        *m_output = std::move(m_tileBytes); // yes, just save its data. We're going to load it synchronously
        return SUCCESS;
        }

    BeAssert(m_tile->IsQueued());
    return m_tile->_Read(m_tileBytes, m_root);
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
    Http::Response response = request.Perform();

    if (Http::ConnectionStatus::OK != response.GetConnectionStatus() || Http::HttpStatus::OK != response.GetHttpStatus())
        {
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

    BeAssert(m_tile->IsQueued());
    if (SUCCESS != m_tile->_Read(m_tileBytes, m_root))
        return ERROR;

    SaveToDb();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Save the data for a 3mx file into the tile cache. Note that this is also called for the scene file.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileData::SaveToDb() const
    {
    auto cache = m_root.GetCache();
    if (!cache.IsValid())
        {
        BeAssert(false);
        return ERROR;
        }

    BeAssert(m_tileBytes.HasData());

    Utf8StringCR name = m_shortName.empty() ? m_fileName : m_shortName;
    CachedStatementPtr stmt;
    cache->GetDb().GetCachedStatement(stmt, "INSERT INTO " TABLE_NAME_TileTree " (Filename,Data,DataSize,Created) VALUES (?,?,?,?)");

    stmt->ClearBindings();
    stmt->BindText(1, name, Statement::MakeCopy::No);
    stmt->BindBlob(2, m_tileBytes.GetData(), (int)m_tileBytes.GetSize(), Statement::MakeCopy::No);
    stmt->BindInt64(3, (int64_t)m_tileBytes.GetSize());

    if (m_tile.IsValid()) // for the root, store NULL for time. That way it will never get purged.
        stmt->BindInt64(4, BeTimeUtilities::GetCurrentTimeAsUnixMillis());

    auto rc = stmt->Step();
    if (BE_SQLITE_DONE != rc)
        {
        BeAssert(false);
        return ERROR;
        }

    cache->ScheduleSave();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
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

    uint64_t garbageSize = sum - m_allowedSize;

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
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::CreateCache()
    {
    if (!IsHttp()) 
        return;

    m_cache = new TileCache();
    if (SUCCESS != m_cache->OpenAndPrepare(m_localCacheName))
        m_cache = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> Root::_RequestFile(Utf8StringCR fileName, StreamBuffer& buffer)
    {
    DgnDb::VerifyClientThread();

    TileData data(fileName, nullptr, *this, &buffer);
    return folly::via(&BeFolly::IOThreadPool::GetPool(), [=](){return data.DoRead();});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> Root::_RequestTile(TileCR tile)
    {
    DgnDb::VerifyClientThread();

    if (!tile.IsNotLoaded()) // this should only be called when the tile is in the "not loaded" state.
        {
        BeAssert(false);
        return ERROR;
        }

    tile.SetIsQueued();
    TileData data(_ConstructTileName(tile), (TileP) &tile, *this, nullptr);
    return folly::via(&BeFolly::IOThreadPool::GetPool(), [=](){return data.DoRead();});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
Root::Root(DgnDbR db, TransformCR location, Utf8CP realityCacheName, Utf8CP rootUrl) : m_db(db), m_rootUrl(rootUrl), m_location(location)
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
    if (LoadState::Ready == m_loadState.load()) // if this node's children are loaded, set them as abandoned too (recursively)
        {
        for (auto const& child : m_children)
            child->SetAbandoned();
        }

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
    if (LoadState::Ready != m_loadState.load()) // children aren't loaded, nothing to do
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

    m_loadState.store(LoadState::NotLoaded);
    m_children.clear();
    }

/*---------------------------------------------------------------------------------**//**
* Draw this node. If it is too coarse, instead draw its children, if they are already loaded.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::VisitComplete Tile::Visit(DrawArgsR args, int depth) const
    {
    bool tooCoarse = true;

    if (IsDisplayable())    // some nodes are merely for structure and don't have any geometry
        {
        Frustum box(m_range);
        args.m_location.Multiply(box.GetPtsP(), box.GetPts(), 8);

        if (FrustumPlanes::Contained::Outside == args.m_context.GetFrustumPlanes().Contains(box))
            {
            _UnloadChildren(args.m_purgeOlderThan);  // this node is completely outside the volume of the frustum. Unload any loaded children if they're expired.
            return VisitComplete::Yes;
            }

        double radius = args.GetTileRadius(*this); // use a sphere to test pixel size. We don't know the orientation of the image within the bounding box.
        DPoint3d center = args.GetTileCenter(*this);
        double pixelSize = radius / args.m_context.GetPixelSizeAtPoint(&center);
        tooCoarse = pixelSize > GetMaximumSize();
        }

    VisitComplete completed = VisitComplete::Yes;
    auto children = _GetChildren(); // returns nullptr if this node's children are not yet valid
    if (tooCoarse && nullptr != children)
        {
        // this node is too coarse for current view, don't draw it and instead draw its children
        m_childrenLastUsed = args.m_now; // save the fact that we've used our children to delay purging them if this node becomes unused

        for (auto const& child : *children)
            {
            if (VisitComplete::Yes != child->Visit(args, depth+1))
                completed = VisitComplete::No;
            }

        if (VisitComplete::Yes == completed)
            return VisitComplete::Yes;
        }
    
    // This node is either fine enough for the current view or has some unloaded children. We'll draw it.
    completed = _Draw(args, depth);

    if (!_HasChildren()) // this is a leaf node - we're done
        return completed;

    if (!tooCoarse && completed==VisitComplete::Yes)
        {
        // This node was fine enough for the current zoom scale and was successfully drawn. If it has loaded children from a previous pass, they're no longer needed.
        _UnloadChildren(args.m_purgeOlderThan);
        return VisitComplete::Yes;
        }

    // this node is too coarse (even though we already drew it) but has unloaded children. Put it into the list of "missing tiles" so we'll draw its children when they're loaded
    if (IsNotLoaded())
        args.m_missing.Insert(depth, this);

    return completed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementAlignedBox3d Tile::ComputeRange() const
    {
    if (m_range.IsValid())
        return m_range;

    auto const* children = _GetChildren(); // only returns fully loaded children
    if (nullptr != children)
        {
        for (auto const& child : *children)
            m_range.UnionOf(m_range, child->ComputeRange()); // this updates the range of the top level node
        }

    return m_range;
    }

/*-----------------------------------------------------------------------------------**//**
* Count the number of tiles for this tile and all of its children.
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
int Tile::CountTiles() const
    {
    int count = 1;

    auto const* children = _GetChildren(); // only returns fully loaded children
    if (nullptr != children)
        {
        for (auto const& child : *children)
            count += child->CountTiles();
        }

    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawArgs::DrawGraphics(ViewContextR context)
    {
    if (m_graphics.m_entries.empty())
        return;

    DEBUG_PRINTF("drawing %d Tiles", m_graphics.m_entries.size());

    ViewFlags flags = context.GetViewFlags();
    flags.SetRenderMode(Render::RenderMode::SmoothShade);
    flags.m_textures = true;
    flags.m_visibleEdges = false;
    flags.m_shadows = false;
    flags.m_ignoreLighting = true;
    m_graphics.SetViewFlags(flags);

    auto branch = m_context.CreateBranch(m_graphics, &m_location);
    
    BeAssert(m_graphics.m_entries.empty()); // CreateBranch should have moved them
    m_context.OutputGraphic(*branch, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawArgs::RequestMissingTiles(RootR root)
    {
    // this requests tiles in depth first order. Could also include distance to frontplane sort too.
    for (auto const& tile : m_missing)
        {
        if (tile.second->IsNotLoaded())
            root._RequestTile(*tile.second);
        }
    }

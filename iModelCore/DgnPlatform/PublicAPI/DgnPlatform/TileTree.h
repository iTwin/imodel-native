/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/TileTree.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/RealityDataCache.h>
#include <folly/futures/Future.h>
#include <Bentley/Tasks/CancellationToken.h>
#include <BeHttp/HttpRequest.h>

#define BEGIN_TILETREE_NAMESPACE    BEGIN_BENTLEY_DGN_NAMESPACE namespace TileTree {
#define END_TILETREE_NAMESPACE      } END_BENTLEY_DGN_NAMESPACE
#define USING_NAMESPACE_TILETREE    using namespace BentleyApi::Dgn::TileTree;

BEGIN_TILETREE_NAMESPACE

/**  @addtogroup GROUP_TileTree TileTree Module

 A TileTree is an abstract class to manage a HLOD (hierarchical level of detail) tree of spatially coherent "3d tiles".

It manages:
 - loading (locally and via http)
 - caching (in memory and on disk)
 - displaying
 - incremental asynchronous refinement

The tree is represented in memory by two classes: TileTree::Root and TileTree::Tile. The tree is in a local
coordinate system, and is positioned in world space by a linear transform.

The HLOD tree can subdivide space using any approach, but must obey the following rules:
 a) there is one TileTree::Root, and it points to the root TileTree::Tile
 b) every Tile in the tree (other than the root tile) has one parent tile
 c) every Tile encloses a volume defined by a tree-axis-aligned DRange3d. That range must be contained within its parent's range.
 d) everything visible in a Tile must also be visible in one of its children (usually at higher detail). That is, either a
    Tile is displayed or its children are displayed; never both.
 e) Tiles may overlap, but generally it works best if they don't.

Display algorithm:
 Starting at the root Tile, intersect the Tile's range (in BIM world coordinates) against the view frustum. If the Tile's range
 is completely outside the view frustum, stop. Otherwise, calculate the size, in pixels, of the Tile by dividing the
 radius of a sphere enclosing the Tile's range by the size (in meters) of the pixel at the center of the Tile. If the pixel size is
 less than the Tile's "maximum pixel size", or if it has no child Tiles, it is displayed. Otherwise, recursively display each of
 its child Tiles.

Tile Loading and Missing Tiles: Tiles cannot be displayed until they are loaded. The root Tile is loaded synchronously,
 so it is always present. Otherwise,  tiles are loaded as they are needed, asynchronously (in another thread.) The status
 of the load process is communicated between threads via the atomic member variable "m_loadState". When a tile is needed
 for a display request but is not loaded,  it is marked as "missing" and is placed in the download queue. Optionally,
 substitute (lower or higher resolution) Tiles may be drawn to show an approximation of the missing tile. Periodically
 (on a timer event, in the main thread), the "_DoProgressive" method of your ProgressiveTask object is called to check
 the list of missing tiles to see if they've arrived. If so, they should be drawn (potentially creating a new set of
 missing tiles). When all tiles have arrived, the ProgressiveTask is removed.

In-memory Tile Caching:
 The strategy for in-memory caching involves purging unused Tiles during _Draw. When any tile is eliminated on range
 or resolution criteria, it and all of its children, recursively, are unloaded if they have not been used recently
 (as defined by the m_expirationTime member of TileTree::Root.) Of course this means that tiles are not purged
 from memory except during _Draw requests.

HTTP request caching:
 TileTree employs the RealityData::Cache class for saving/loading copies of tile data from HTTP request in a SQLite database in
 the temporary directory. When an HTTP-based tile is needed, the local cache is first checked and used if present. Otherwise
 an HTTP GET request is issued and the result is saved on arrival. The local cached is purged periodically so it doesn't
 grow beyond the "maxSize" argument passed to TileTree::Root::CreateCache. Tiles are always loaded via the TileLoader::_LoadTile
 method regardless of whether their data comes from a local file, an HTTP request, or from the local cache.

*/

DEFINE_POINTER_SUFFIX_TYPEDEFS(DrawArgs)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Tile)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Root)
DEFINE_POINTER_SUFFIX_TYPEDEFS(TileLoader)

DEFINE_REF_COUNTED_PTR(Tile)
DEFINE_REF_COUNTED_PTR(Root)
DEFINE_REF_COUNTED_PTR(TileLoader)

typedef std::chrono::steady_clock::time_point TimePoint;
typedef std::shared_ptr<struct LoadState> LoadStatePtr;

//=======================================================================================
//! A ByteStream with a "current position". Used for reading tiles
// @bsiclass                                                    Keith.Bentley   03/16
//=======================================================================================
struct StreamBuffer : ByteStream
    {
    uint32_t m_currPos = 0;
    ByteCP GetCurrent() const {return (m_currPos > GetSize()) ? nullptr : GetData() + m_currPos;}
    ByteCP Advance(uint32_t size) {m_currPos += size; return GetCurrent();} // returns nullptr if advanced past end.
    void SetPos(uint32_t pos) {m_currPos=pos;}
    StreamBuffer() {}
    StreamBuffer(ByteStream const& other) : ByteStream(other) {}
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   09/16
//=======================================================================================
struct LoadState : Tasks::ICancellationToken, NonCopyableClass
{
    BeAtomic<bool> m_canceled;
    BeAtomic<int> m_requested;
    BeAtomic<int> m_fromHttp;
    BeAtomic<int> m_fromFile;
    BeAtomic<int> m_fromDb;
    DGNPLATFORM_EXPORT ~LoadState();
    bool IsCanceled() override {return m_canceled.load();}
    void SetCanceled() {m_canceled.store(true);}
    void Register(std::weak_ptr<Tasks::ICancellationListener> listener) override {}
    void Reset() {m_fromDb.store(0); m_fromHttp.store(0); m_fromFile.store(0);}
};

//=======================================================================================
//! A Tile in a TileTree. Every Tile has 0 or 1 parent Tile and 0 or more child Tiles. 
//! The range member is an ElementAlignedBox in the local coordinate system of the TileTree.
//! All child Tiles must be contained within the range of their parent Tile.
// @bsiclass                                                    Keith.Bentley   09/16
//=======================================================================================
struct Tile : RefCountedBase, NonCopyableClass
{
    friend struct Root;
    enum class LoadStatus : int {NotLoaded=0, Queued=1, Loading=2, Ready=3, NotFound=4, Abandoned=5};
    typedef bvector<TilePtr> ChildTiles;

protected:
    RootR   m_root;
    mutable ElementAlignedBox3d m_range;
    TileCP m_parent;
    mutable BeAtomic<LoadStatus> m_loadStatus;
    mutable ChildTiles m_children;
    mutable TimePoint m_childrenLastUsed; //! updated whenever this tile is used for display

    void SetAbandoned() const;

public:
    Tile(RootR root, TileCP parent) : m_root(root), m_parent(parent), m_loadStatus(LoadStatus::NotLoaded) {}
    DGNPLATFORM_EXPORT void ExtendRange(DRange3dCR childRange) const;
    double GetRadius() const {return 0.5 * m_range.low.Distance(m_range.high);}
    DPoint3d GetCenter() const {return DPoint3d::FromInterpolate(m_range.low, .5, m_range.high);}
    ElementAlignedBox3d GetRange() const {return m_range;}
    DGNPLATFORM_EXPORT void Draw(DrawArgsR, int depth) const;
    LoadStatus GetLoadStatus() const {return (LoadStatus) m_loadStatus.load();}
    void SetIsReady() {return m_loadStatus.store(LoadStatus::Ready);}
    void SetIsQueued() const {return m_loadStatus.store(LoadStatus::Queued);}
    void SetNotLoaded() {return m_loadStatus.store(LoadStatus::NotLoaded);}
    void SetNotFound() {return m_loadStatus.store(LoadStatus::NotFound);}
    bool IsQueued() const {return m_loadStatus.load() == LoadStatus::Queued;}
    bool IsAbandoned() const {return m_loadStatus.load() == LoadStatus::Abandoned;}
    bool IsReady() const {return m_loadStatus.load() == LoadStatus::Ready;}
    bool IsNotLoaded() const {return m_loadStatus.load() == LoadStatus::NotLoaded;}
    bool IsNotFound() const {return m_loadStatus.load() == LoadStatus::NotFound;}
    bool IsDisplayable() const {return _GetMaximumSize() > 0.0;}
    TileCP GetParent() const {return m_parent;}
    RootCR GetRoot() const {return m_root;}
    RootR GetRootR() {return m_root;}
    DGNPLATFORM_EXPORT int CountTiles() const; //! for debugging
    DGNPLATFORM_EXPORT ElementAlignedBox3d ComputeRange() const;

    virtual void _OnChildrenUnloaded() const {}
    DGNPLATFORM_EXPORT virtual void _UnloadChildren(TimePoint olderThan) const;

    //! Determine whether this tile has any child tiles. Return true even if the children are not yet created.
    virtual bool _HasChildren() const = 0;

    //! Get the array of children for this Tile.
    //! @param[in] create If false, return nullptr if this tile has children but they are not yet created. Otherwise create them now.
    virtual ChildTiles const* _GetChildren(bool create) const = 0;

    //! Draw the Graphics of this Tile into args.
    //! @param[in] args The DrawArgs for the current display request.
    //! @param[in] depth The depth of this tile in the tree. This is necessary to sort missing tiles depth-first.
    virtual void _DrawGraphics(DrawArgsR args, int depth) const = 0;

    //! Called when tile data is required. The loader will be added to the IOPool and will execute asynchronously.
    virtual TileLoaderPtr _CreateTileLoader(LoadStatePtr) = 0;

    //! Get the name of this Tile.
    virtual Utf8String _GetTileName() const = 0;

    //! Get the maximum size, in pixels, that this Tile should occupy on the screen. If larger, use its children, if possible.
    virtual double _GetMaximumSize() const = 0;
};

/*=================================================================================**//**
//! The root of a tree of tiles. This object stores the location of the tree relative to world coordinates. 
//! It also facilitates local caching of HTTP-based tiles.
// @bsiclass                                                    Keith.Bentley   03/16
+===============+===============+===============+===============+===============+======*/
struct Root : RefCountedBase, NonCopyableClass
{
protected:
    bool m_isHttp = false;
    bool m_pickable = false;
    int m_activeLoads = 0;
    DgnDbR m_db;
    BeFileName m_localCacheName;
    Transform m_location;
    TilePtr m_rootTile;
    Utf8String m_rootUrl;
    Utf8String m_rootDir;
    std::chrono::seconds m_expirationTime = std::chrono::seconds(20); // save unused tiles for 20 seconds
    Dgn::Render::SystemP m_renderSystem = nullptr;
    RealityData::CachePtr m_cache;
    BeConditionVariable m_cv;

    //! Clear the current tiles and wait for all pending download requests to complete/abort.
    //! All subclasses of Root must call this method in their destructor. This is necessary, since it must be called while the subclass vtable is 
    //! still valid and that cannot be accomplished in the destructor of Root.
    DGNPLATFORM_EXPORT void ClearAllTiles(); 

public:
    DGNPLATFORM_EXPORT virtual folly::Future<BentleyStatus> _RequestTile(TileR tile, LoadStatePtr loads);

    ~Root() {BeAssert(!m_rootTile.IsValid());} // NOTE: Subclasses MUST call ClearAllTiles in their destructor!
    void StartTileLoad() {BeMutexHolder holder(m_cv.GetMutex()); ++m_activeLoads;}
    void DoneTileLoad() {{BeMutexHolder holder(m_cv.GetMutex()); --m_activeLoads; BeAssert(m_activeLoads>=0);} m_cv.notify_all();}
    void WaitForAllLoads() {BeMutexHolder holder(m_cv.GetMutex()); while (m_activeLoads>0) m_cv.InfiniteWait(holder);}
    bool IsHttp() const {return m_isHttp;}
    bool IsPickable() const {return m_pickable;}
    void SetPickable(bool pickable) {m_pickable = pickable;}
    TransformCR GetLocation() const {return m_location;}
    void SetLocation(TransformCR location) {m_location = location;}
    RealityData::CachePtr GetCache() const {return m_cache;}
    TilePtr GetRootTile() const {return m_rootTile;} //!< Get the root Tile of this Root
    DgnDbR GetDgnDb() const {return m_db;} //!< Get the DgnDb from which this Root was created.
    Dgn::Render::SystemP GetRenderSystem() const {return m_renderSystem;}
    ElementAlignedBox3d ComputeRange() const {return m_rootTile->ComputeRange();}

    //! Get the full name of a Tile in this TileTree. By default it concatenates the tile name to the rootDir
    virtual Utf8String _ConstructTileName(TileCR tile) const {return m_rootDir + tile._GetTileName();}

    //! Ctor for Root.
    //! @param db The DgnDb from which this Root was created. This is needed to get the Units().GetDgnGCS()
    //! @param location The transform from tile coordinates to BIM world coordinates.
    //! @param rootUrl The root url for loading tiles. This name will be prepended to tile names.
    //! @param system The Rendering system used to create Render::Graphic used to draw this TileTree.
    DGNPLATFORM_EXPORT Root(DgnDbR db, TransformCR location, Utf8CP rootUrl, Dgn::Render::SystemP system);

    //! Set expiration time for unused Tiles. During calls to Draw, unused tiles that haven't been used for this number of seconds will be purged.
    void SetExpirationTime(std::chrono::seconds val) {m_expirationTime = val;}

    //! Get expiration time for unused Tiles, in seconds.
    std::chrono::seconds GetExpirationTime() const {return m_expirationTime;}

    //! Create a RealityData::Cache for Tiles from this Root. This will either create or open the SQLite file holding locally cached previously-downloaded versions of Tiles.
    //! @param realityCacheName The name of the reality cache database file, relative to the temporary directory.
    //! @param maxSize The cache maximum size in bytes.
    //! @note If this is not an HTTP root, this does nothing.
    DGNPLATFORM_EXPORT void CreateCache(Utf8CP realityCacheName, uint64_t maxSize);

    //! Delete, if present, the local SQLite file holding the cache of downloaded tiles.
    //! @note This will first delete the local RealityData::Cache, aborting all outstanding requests and waiting for the queue to empty.
    DGNPLATFORM_EXPORT BentleyStatus DeleteCacheFile();

    //! Traverse the tree and draw the appropriate set of tiles that intersect the view frustum.
    //! @note during the traversal, previously loaded but now unused tiles are purged if they are expired.
    //! @note This method must be called from the client thread
    void Draw(DrawArgs& args) {m_rootTile->Draw(args, 0);}
};

//=======================================================================================
//! This object is created to read and load a single tile asynchronously. 
//! If caching is enabled, it will first attempt to read the data from the cache. If it's
//! not available it will call _ReadFromSource(). Once the data is available, _LoadTile is called. 
//! All methods of this class might be called from worker threads except for the constructor 
//! which is guaranteed to run on the client thread. If something you required is not thread 
//! safe you must capture it during construction.
// @bsiclass                                                    Mathieu.Marchand  11/2016
//=======================================================================================
struct TileLoader : RefCountedBase, NonCopyableClass
{
protected:
    Utf8String m_fileName;      // full file or URL name
    TilePtr m_tile;             // tile to load, cannot be null.
    Utf8String m_cacheKey;      // for loading or saving to tile cache
    StreamBuffer m_tileBytes;   // when available, bytes are saved here
    LoadStatePtr m_loads;

    //! Constructor for TileLoader.
    //! @param[in] fileName full file name or URL name.
    //! @param[in] tile The tile that we are loading.
    //! @param[in] loads The cancellation token.
    //! @param[in] cacheKey The tile unique name use for caching. Might be empty if caching is not required.
    TileLoader(Utf8StringCR fileName, TileR tile, LoadStatePtr& loads, Utf8StringCR cacheKey)
        : m_fileName(fileName), m_tile(&tile), m_loads(loads), m_cacheKey(cacheKey) {}

    BentleyStatus LoadTile();
    DGNPLATFORM_EXPORT virtual BentleyStatus _SaveToDb();
    DGNPLATFORM_EXPORT virtual BentleyStatus _ReadFromDb();

    //! Called from worker threads to get the data from the original location. E.g. disk, web, or created locally.
    DGNPLATFORM_EXPORT virtual folly::Future<BentleyStatus> _GetFromSource();

    //! Load tile. This method is called when the tile data becomes available, regardless of the source of the data.
    virtual BentleyStatus _LoadTile() = 0; 

public:
    struct LoadFlag
        {
        Root& m_root;
        LoadFlag(Root& root) : m_root(root) {root.StartTileLoad();}
        ~LoadFlag() {m_root.DoneTileLoad();}
        };

    //! Called from worker threads to create a tile. Could be from cache or from source.
    folly::Future<BentleyStatus> CreateTile();
};

//=======================================================================================
// @bsiclass                                                   Mathieu.Marchand  11/2016
//=======================================================================================
struct HttpDataQuery
{
    Http::HttpByteStreamBodyPtr m_responseBody;
    Http::Request m_request;
    Http::Response m_response;
    LoadStatePtr m_loads;

    DGNPLATFORM_EXPORT HttpDataQuery(Utf8StringCR url, LoadStatePtr loads);

    bool WasCanceled() const {return m_response.GetConnectionStatus() == Http::ConnectionStatus::Canceled;}

    Http::Request& GetRequest() {return m_request;}

    //! Valid only after a call to perform.
    Http::Response& GetResponse() {return m_response;}

    //! Perform http request and wait for the result.
    DGNPLATFORM_EXPORT BentleyStatus Perform(ByteStream& data);
};

//=======================================================================================
// @bsiclass                                                   Mathieu.Marchand  11/2016
//=======================================================================================
struct FileDataQuery
{
    Utf8String m_fileName;
    LoadStatePtr m_loads;

    FileDataQuery(Utf8StringCR fileName, LoadStatePtr loads) : m_fileName(fileName), m_loads(loads) {}

    //! Read the entire file in a single chunk of memory.
    DGNPLATFORM_EXPORT BentleyStatus Perform(ByteStream& data);
};

//=======================================================================================
//! Arguments for drawing a tile. As tiles are drawn, their Render::Graphics go into the GraphicBranch member of this object. After all
//! in-view tiles are drawn, the accumulated list of Render::Graphics are placed in a GraphicBranch with the location
//! transform for the scene (that is, the tile graphics are always in the local coordinate system of the TileTree.)
//! If higher resolution tiles are needed but missing, the graphics for lower resolution tiles are
//! drawn and the missing tiles are requested for download (if necessary.) They are then added to the MissingNodes member. If the
//! MissingNodes list is not empty, schedule a ProgressiveDisplay that checks for the arrival of the missing nodes and draws them (using
//! this class). Each iteration of ProgressiveDisplay starts with a list of previously-missing tiles and generates a new list of
//! still-missing tiles until all have arrived, or the view changes externally.
// @bsiclass                                                    Keith.Bentley   05/16
//=======================================================================================
struct DrawArgs
{
    typedef bmultimap<int, TileCPtr> MissingNodes;
    RenderContextR m_context;
    Transform m_location;
    double m_scale;
    Render::GraphicBranch m_graphics;
    Render::GraphicBranch m_substitutes;
    MissingNodes m_missing;
    TimePoint m_now;
    TimePoint m_purgeOlderThan;
    ClipVectorCP m_clip;

    DPoint3d GetTileCenter(TileCR tile) const {return DPoint3d::FromProduct(m_location, tile.GetCenter());}
    double GetTileRadius(TileCR tile) const {return m_scale * tile.GetRadius();}
    void SetClip(ClipVectorCP clip) {m_clip = clip;}
    DrawArgs(RenderContextR context, TransformCR location, TimePoint now, TimePoint purgeOlderThan, ClipVectorCP clip = nullptr) : m_context(context), m_location(location), m_now(now), m_purgeOlderThan(purgeOlderThan), m_clip(clip) {m_scale = location.ColumnXMagnitude();}
    DGNPLATFORM_EXPORT void DrawGraphics(ViewContextR); // place all entries into a GraphicBranch and send it to the ViewContext.
    DGNPLATFORM_EXPORT void RequestMissingTiles(RootR, LoadStatePtr);
};

//=======================================================================================
//! A QuadTree is a 2d TileTree that subdivides each tile into 4 child equal-sized tiles, each with one corner at the center of its parent.
//! A tile in a QuadTree can be addressed by a TileId comprised of a level (depth) and a row/column numbers. 
//! The root tile is {0,0,0} and it is not necessarily square.
// @bsiclass                                                    Keith.Bentley   11/16
//=======================================================================================
namespace QuadTree
{
//=======================================================================================
//! Identifies a tile in a QuadTree
// @bsiclass                                                    Keith.Bentley   08/16
//=======================================================================================
struct TileId
{
    uint8_t m_level;
    uint32_t m_row;
    uint32_t m_column;
    TileId() {}
    TileId(uint8_t level, uint32_t col, uint32_t row) {m_level=level; m_column=col; m_row=row;}
};

//=======================================================================================
//! The root of a QuadTree
// @bsiclass                                                    Keith.Bentley   08/16
//=======================================================================================
struct Root : TileTree::Root
{
    DEFINE_T_SUPER(TileTree::Root)

    ColorDef m_tileColor;      //! for setting transparency
    uint8_t m_maxZoom;         //! the maximum zoom level for this map
    uint32_t m_maxPixelSize;   //! the maximum size, in pixels, that the radius of the diagonal of the tile should stretched to. If the tile's size on screen is larger than this, use its children.

    uint32_t GetMaxPixelSize() const {return m_maxPixelSize;}
    Root(DgnDbR, TransformCR location, Utf8CP rootUrl, Render::SystemP system, uint8_t maxZoom, uint32_t maxSize, double transparency=0.0);
    void DrawInView(RenderContextR context);
};
    
//=======================================================================================
//! A QuadTree tile. May or may not have its graphics present.
// @bsiclass                                                    Keith.Bentley   05/16
//=======================================================================================
struct Tile : TileTree::Tile
{
    DEFINE_T_SUPER(TileTree::Tile)

    TileId m_id; 
    Render::IGraphicBuilder::TileCorners m_corners; 
    Render::GraphicPtr m_graphic;                   

    Tile(Root& quadRoot, TileId id, Tile const* parent) : T_Super(quadRoot, parent), m_id(id) {}

    TileId GetTileId() const {return m_id;}
    virtual TilePtr _CreateChild(TileId) const = 0;
    bool TryLowerRes(TileTree::DrawArgsR args, int depth) const;
    void TryHigherRes(TileTree::DrawArgsR args) const;
    bool _HasChildren() const override {return m_id.m_level < GetQuadRoot().m_maxZoom;}
    bool HasGraphics() const {return IsReady() && m_graphic.IsValid();}
    ChildTiles const* _GetChildren(bool load) const override;
    void _DrawGraphics(TileTree::DrawArgsR, int depth) const override;
    Utf8String _GetTileName() const override {return Utf8PrintfString("%d/%d/%d", m_id.m_level, m_id.m_column, m_id.m_row);}
    Root& GetQuadRoot() const {return (Root&) m_root;}
    double _GetMaximumSize() const override {return GetQuadRoot().GetMaxPixelSize();}
};

//=======================================================================================
// The ProgressiveTask for drawing QuadTree tiles as they arrive asynchronously.
// @bsiclass                                                    Keith.Bentley   05/16
//=======================================================================================
struct ProgressiveTask : Dgn::ProgressiveTask
{
    Root& m_root;
    DrawArgs::MissingNodes m_missing;
    TimePoint m_nextShow;
    LoadStatePtr m_loads;

    Completion _DoProgressive(ProgressiveContext& context, WantShow&) override;
    ProgressiveTask(Root& root, DrawArgs::MissingNodes& nodes, LoadStatePtr loads) : m_root(root), m_missing(std::move(nodes)), m_loads(loads) {}
    ~ProgressiveTask() {if (nullptr != m_loads) m_loads->SetCanceled();}
};

}

END_TILETREE_NAMESPACE


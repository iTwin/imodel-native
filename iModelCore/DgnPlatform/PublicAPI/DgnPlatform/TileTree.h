/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/TileTree.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnPlatform.h"
#include <folly/futures/Future.h>
#include <Bentley/Tasks/CancellationToken.h>
#include <BeHttp/HttpRequest.h>
#include <DgnPlatform/RealityDataCache.h>

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
DEFINE_POINTER_SUFFIX_TYPEDEFS(PickArgs)
DEFINE_POINTER_SUFFIX_TYPEDEFS(MissingNode)
DEFINE_POINTER_SUFFIX_TYPEDEFS(MissingNodes)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Tile)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Root)
DEFINE_POINTER_SUFFIX_TYPEDEFS(TileLoader)
DEFINE_POINTER_SUFFIX_TYPEDEFS(DirtyRanges)

DEFINE_REF_COUNTED_PTR(Tile)
DEFINE_REF_COUNTED_PTR(Root)
DEFINE_REF_COUNTED_PTR(TileLoader)

typedef std::shared_ptr<struct TileLoadState> TileLoadStatePtr;

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
    uint32_t GetPos() const {return m_currPos;}
    DGNPLATFORM_EXPORT bool ReadBytes(void* buf, uint32_t size);

    StreamBuffer() {}
    StreamBuffer(ByteStream const& other) : ByteStream(other) {}
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   09/16
//=======================================================================================
struct TileLoadState : Tasks::ICancellationToken, NonCopyableClass
{
private:
    TileCPtr        m_tile;
    BeAtomic<bool>  m_canceled;
public:
    explicit TileLoadState(TileCR tile) : m_tile(&tile) { }
    DGNPLATFORM_EXPORT ~TileLoadState();
    bool IsCanceled() override {return m_canceled.load();}
    void SetCanceled() {m_canceled.store(true);}
    void Register(std::weak_ptr<Tasks::ICancellationListener> listener) override {}
    TileCR GetTile() const { return *m_tile; }

    struct PtrComparator
    {
        using is_transparent = std::true_type;

        bool operator()(TileLoadStatePtr const& lhs, TileLoadStatePtr const& rhs) const { return operator()(lhs->m_tile.get(), rhs->m_tile.get()); }
        bool operator()(TileLoadStatePtr const& lhs, TileCP rhs) const { return operator()(lhs->m_tile.get(), rhs); }
        bool operator()(TileCP lhs, TileLoadStatePtr const& rhs) const { return operator()(lhs, rhs->m_tile.get()); }
        bool operator()(TileCP lhs, TileCP rhs) const { return lhs < rhs; }
    };
};

//=======================================================================================
//! A set of ranges within a TileTree which have become damaged due to modification of
//! the model. Can be partitioned to include only those damaged ranges which intersect
//! a given range.
// @bsistruct                                                   Paul.Connelly   05/17
//=======================================================================================
struct DirtyRanges
{
    typedef bvector<DRange3d> List;
    using iterator = List::iterator;
    using const_iterator = List::const_iterator;
private:
    iterator    m_begin;
    iterator    m_end;
public:
    DirtyRanges(iterator begin, iterator end) : m_begin(begin), m_end(end) { }
    explicit DirtyRanges(List& list) : m_begin(list.begin()), m_end(list.end()) { }
    
    //! Returns the set of dirty ranges which intersect the given range.
    //! Note that this modifies the ordering of the underlying List's contents (but only within the range thereof represented by this object).
    DGNPLATFORM_EXPORT DirtyRanges Intersect(DRange3dCR range) const;

    bool empty() const { return m_begin == m_end; }
    size_t size() const { return std::distance(m_begin, m_end); }
    const_iterator begin() const { return m_begin; }
    const_iterator end() const { return m_end; }
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
    TileCP  m_parent;
    int     m_depth;
    mutable BeAtomic<LoadStatus> m_loadStatus;
    mutable ChildTiles m_children;
    mutable BeTimePoint m_childrenLastUsed; //! updated whenever this tile is used for display

    void SetAbandoned() const;
    void UnloadChildren(BeTimePoint olderThan) const;

    //! Mark this tile as invalidated, e.g. because its contents have been modified.
    virtual void _Invalidate() = 0;
public:
    Tile(RootR root, TileCP parent) : m_root(root), m_parent(parent), m_depth(nullptr==parent ? 0 : parent->GetDepth()+1), m_loadStatus(LoadStatus::NotLoaded) {}
    DGNPLATFORM_EXPORT void ExtendRange(DRange3dCR childRange) const;
    double GetRadius() const {return 0.5 * m_range.low.Distance(m_range.high);}
    int GetDepth() const {return m_depth;}
    DPoint3d GetCenter() const {return DPoint3d::FromInterpolate(m_range.low, .5, m_range.high);}
    ElementAlignedBox3d const& GetRange() const {return m_range;}
    DGNPLATFORM_EXPORT void Pick(PickArgsR args, int depth) const;
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
    DGNPLATFORM_EXPORT virtual void _UnloadChildren(BeTimePoint olderThan) const;

    //! Determine whether this tile has any child tiles. Return true even if the children are not yet created.
    virtual bool _HasChildren() const = 0;

    //! Returns whether this tile has graphics.
    virtual bool _HasGraphics() const = 0;

    //! Get the array of children for this Tile.
    //! @param[in] create If false, return nullptr if this tile has children but they are not yet created. Otherwise create them now.
    virtual ChildTiles const* _GetChildren(bool create) const = 0;

    //! Draw the Graphics of this Tile into args.
    //! @param[in] args The DrawArgs for the current display request.
    virtual void _DrawGraphics(DrawArgsR args) const = 0;

    virtual void _PickGraphics(PickArgsR args, int depth) const {}

    //! Called when tile data is required.
    virtual TileLoaderPtr _CreateTileLoader(TileLoadStatePtr) = 0;

    //! Get the tile cache key for this Tile.
    virtual Utf8String _GetTileCacheKey() const = 0;

    //! Get the maximum size, in pixels, that this Tile should occupy on the screen. If larger, use its children, if possible.
    virtual double _GetMaximumSize() const = 0;

    //! Describes a Tile's visibility within a viewing frustum
    enum class Visibility
    {
        OutsideFrustum, // this tile is entirely outside of the viewing frustum
        TooCoarse, // this tile is too coarse to be drawn
        Visible, // this tile is of the correct size to be drawn
    };

    //! Compute the visibility of this tile
    DGNPLATFORM_EXPORT Visibility GetVisibility(DrawArgsCR args) const;

    enum class SelectParent { Yes, No };

    //! Populates a list of tiles to draw. Returns SelectParent::Yes to substitute this tile's parent in its place.
    DGNPLATFORM_EXPORT virtual SelectParent _SelectTiles(bvector<TileCPtr>& selected, DrawArgsR args) const;

    //! Returns true if this tile is entirely outside of the viewing frustum or clipping planes
    bool IsCulled(DrawArgsCR args) const;

    void Invalidate(DirtyRangesCR dirty);
};

/*=================================================================================**//**
//! The root of a tree of tiles. This object stores the location of the tree relative to world coordinates. 
//! It also facilitates local caching of HTTP-based tiles.
// @bsiclass                                                    Keith.Bentley   03/16
+===============+===============+===============+===============+===============+======*/
struct Root : RefCountedBase, NonCopyableClass
{
    friend struct DrawArgs;

protected:
    bool m_isHttp;
    bool m_pickable = false;
    mutable std::set<TileLoadStatePtr, TileLoadState::PtrComparator> m_activeLoads;
    DgnDbR m_db;
    BeFileName m_localCacheName;
    Transform m_location;         // transform from tile coordinates to world coordinates
    TilePtr m_rootTile;
    Utf8String m_rootResource;  // either directory or Url.
    BeDuration m_expirationTime = BeDuration::Seconds(20); // save unused tiles for 20 seconds
    Dgn::Render::SystemP m_renderSystem = nullptr;
    RealityData::CachePtr m_cache;
    mutable BeConditionVariable m_cv;
    DirtyRanges::List m_damagedRanges;

    //! Clear the current tiles and wait for all pending download requests to complete/abort.
    //! All subclasses of Root must call this method in their destructor. This is necessary, since it must be called while the subclass vtable is 
    //! still valid and that cannot be accomplished in the destructor of Root.
    DGNPLATFORM_EXPORT void ClearAllTiles(); 

    virtual ClipVectorCP _GetClipVector() const { return nullptr; } // clip vector used by DrawArgs when rendering
    virtual Transform _GetTransform(RenderContextR context) const { return GetLocation(); } // transform used by DrawArgs when rendering

    void InvalidateDamagedTiles();
public:
    DGNPLATFORM_EXPORT virtual folly::Future<BentleyStatus> _RequestTile(TileR tile, TileLoadStatePtr loads);
    void RequestTiles(MissingNodesCR);

    ~Root() {BeAssert(!m_rootTile.IsValid());} // NOTE: Subclasses MUST call ClearAllTiles in their destructor!
    void StartTileLoad(TileLoadStatePtr) const;
    void DoneTileLoad(TileLoadStatePtr) const;
    void CancelTileLoad(TileCR tile);
    void WaitForAllLoads() {BeMutexHolder holder(m_cv.GetMutex()); while (m_activeLoads.size()>0) m_cv.InfiniteWait(holder);}
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
    DGNPLATFORM_EXPORT void MarkDamaged(DRange3dCR range);

    //! Get the resource name (file name or URL) of a Tile in this TileTree. By default it concatenates the tile cache key to the rootResource
    virtual Utf8String _ConstructTileResource(TileCR tile) const {return m_rootResource + tile._GetTileCacheKey();}

    //! Get the name of this tile tree, chiefly for debugging
    virtual Utf8CP _GetName() const = 0;

    //! Specify any view flags which should be overridden when rendering this tile tree.
    DGNPLATFORM_EXPORT virtual Render::ViewFlagsOverrides _GetViewFlagsOverrides() const;

    //! Ctor for Root.
    //! @param db The DgnDb from which this Root was created. This is needed to call GeoLocation().GetDgnGCS()
    //! @param location The transform from tile coordinates to BIM world coordinates.
    //! @param rootResource The root resource (directory or Url) from which tiles can be loaded, or nullptr if the Url's are generated by _ConstructTileResource
    //! @param system The Rendering system used to create Render::Graphic used to draw this TileTree.
    DGNPLATFORM_EXPORT Root(DgnDbR db, TransformCR location, Utf8CP rootResource, Dgn::Render::SystemP system);

    //! Set expiration time for unused Tiles. During calls to Draw, unused tiles that haven't been used for this number of seconds will be purged.
    void SetExpirationTime(BeDuration val) {m_expirationTime = val;}

    //! Get expiration time for unused Tiles.
    BeDuration GetExpirationTime() const {return m_expirationTime;}
                                                                                                                                           
    //! Create a RealityData::Cache for Tiles from this Root. This will either create or open the SQLite file holding locally cached previously-downloaded versions of Tiles.
    //! @param realityCacheName The name of the reality cache database file, relative to the temporary directory.
    //! @param maxSize The cache maximum size in bytes.
    //! @param httpOnly If true create cache only if HTTP root.
    //! @note If httpOnly && this is not an HTTP root, this does nothing.
    DGNPLATFORM_EXPORT void CreateCache(Utf8CP realityCacheName, uint64_t maxSize, bool httpOnly = true);

    //! Delete, if present, the local SQLite file holding the cache of downloaded tiles.
    //! @note This will first delete the local RealityData::Cache, aborting all outstanding requests and waiting for the queue to empty.
    DGNPLATFORM_EXPORT BentleyStatus DeleteCacheFile();

    //! Traverse the tree and draw the appropriate set of tiles that intersect the view frustum.
    //! @note Tiles which should be drawn but which are not yet available will be scheduled for progressive display.
    //! @note During the traversal, previously loaded but now unused tiles are purged if they are expired.
    //! @note This method must be called from the client thread
    void DrawInView(RenderContextR context);

    //! Perform a pick operation on the contents of this tree
    DGNPLATFORM_EXPORT void Pick(PickContext& context, TransformCR location, ClipVectorCP);
};

//=======================================================================================
//! This object is created to read and load a single tile asynchronously. 
//! If caching is enabled, it will first attempt to read the data from the cache. If it's
//! not available it will call _GetFromSource(). Once the data is available, _LoadTile is called. 
//! All methods of this class might be called from worker threads except for the constructor 
//! which is guaranteed to run on the client thread. If something you required is not thread 
//! safe you must capture it during construction.
// @bsiclass                                                    Mathieu.Marchand  11/2016
//=======================================================================================
struct TileLoader : RefCountedBase, NonCopyableClass
{
protected:
    Utf8String m_resourceName;  // full file or URL name
    TilePtr m_tile;             // tile to load, cannot be null.
    TileLoadStatePtr m_loads;

    // Cacheable information
    Utf8String m_cacheKey;      // for loading or saving to tile cache
    StreamBuffer m_tileBytes;   // when available, bytes are saved here
    Utf8String m_contentType;   // MIME type of the data. Can be empty.
    uint64_t m_expirationDate;  // Expiration date. Will be 0 when not available.
    bool m_saveToCache = false;

    //! Constructor for TileLoader.
    //! @param[in] resourceName full file name or URL name.
    //! @param[in] tile The tile that we are loading.
    //! @param[in] loads The cancellation token.
    //! @param[in] cacheKey The tile unique name use for caching. Might be empty if caching is not required.
    TileLoader(Utf8StringCR resourceName, TileR tile, TileLoadStatePtr& loads, Utf8StringCR cacheKey)
        : m_resourceName(resourceName), m_tile(&tile), m_loads(loads), m_cacheKey(cacheKey), m_expirationDate(0) {}

    BentleyStatus LoadTile();
    BentleyStatus DoReadFromDb();
    BentleyStatus DoSaveToDb();

public:
    bool IsCanceledOrAbandoned() const {return (m_loads != nullptr && m_loads->IsCanceled()) || m_tile->IsAbandoned();}
    Dgn::Render::SystemP GetRenderSystem() { return m_tile->GetRoot().GetRenderSystem(); }

    DGNPLATFORM_EXPORT virtual folly::Future<BentleyStatus> _SaveToDb();
    DGNPLATFORM_EXPORT virtual folly::Future<BentleyStatus> _ReadFromDb();

    //! Called to get the data from the original location. The call must be fast and execute any long running operation asynchronously.
    DGNPLATFORM_EXPORT virtual folly::Future<BentleyStatus> _GetFromSource();

    //! Load tile. This method is called when the tile data becomes available, regardless of the source of the data. Called from worker threads.
    virtual BentleyStatus _LoadTile() = 0; 

    struct LoadFlag
        {
        TileLoadStatePtr m_state;
        LoadFlag(TileLoadStatePtr state) : m_state(state) {state->GetTile().GetRoot().StartTileLoad(state);}
        ~LoadFlag() {m_state->GetTile().GetRoot().DoneTileLoad(m_state);}
        };

    //! Perform the load asynchronously.
    folly::Future<BentleyStatus> Perform();
};

//=======================================================================================
// @bsiclass                                                   Mathieu.Marchand  11/2016
//=======================================================================================
struct HttpDataQuery
{
    Http::HttpByteStreamBodyPtr m_responseBody;
    Http::Request m_request;
    TileLoadStatePtr m_loads;

    DGNPLATFORM_EXPORT HttpDataQuery(Utf8StringCR url, TileLoadStatePtr loads);

    Http::Request& GetRequest() {return m_request;}

    //! Parse expiration date from the response. Return false if caching is not allowed.
    DGNPLATFORM_EXPORT static bool GetCacheContolExpirationDate(uint64_t& expiration, Http::Response const& response);

    //! Valid only after 'Perform' has completed.
    ByteStream const& GetData() const {return m_responseBody->GetByteStream();}
    
    //! Perform http request and wait for the result.
    DGNPLATFORM_EXPORT folly::Future<Http::Response> Perform();
};

//=======================================================================================
// @bsiclass                                                   Mathieu.Marchand  11/2016
//=======================================================================================
struct FileDataQuery
{
    Utf8String m_fileName;
    TileLoadStatePtr m_loads;

    FileDataQuery(Utf8StringCR fileName, TileLoadStatePtr loads) : m_fileName(fileName), m_loads(loads) {}

    //! Read the entire file in a single chunk of memory.
    DGNPLATFORM_EXPORT folly::Future<ByteStream> Perform();
};

//=======================================================================================
//! Describes a missing tile and its distance from the camera.
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct MissingNode
{
private:
    TileCPtr    m_tile;
    double      m_distance;
public:
    MissingNode(TileCR tile, double distance) : m_tile(&tile), m_distance(distance) {}
    MissingNode() {} // for bset use only...

    TileCR GetTile() const { return *m_tile; }
    double GetDistance() const { return m_distance; }
    int GetDepth() const { return GetTile().GetDepth(); }

    bool operator<(MissingNodeCR rhs) const
        {
        if (m_tile.get() == rhs.m_tile.get())
            return false;
        else if (GetDistance() != rhs.GetDistance())
            return GetDistance() < rhs.GetDistance();
        else
            return GetDepth() < rhs.GetDepth();
        }
};

//=======================================================================================
//! A set of missing tiles intended to be queued for loading.
//! Sorted according to priority for loading - tiles nearer the root or closer to the camera
//! are loaded with priority.
//! A given tile can only appear in the set once.
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct MissingNodes
{
private:
    typedef bset<MissingNode> Set;

    Set m_set;
public:
    void Insert(TileCR tile, double distance);
    bool Contains(TileCR tile) const;

    bool empty() const { return m_set.empty(); }
    size_t size() const { return m_set.size(); }
    void clear() { m_set.clear(); }

    typedef Set::const_iterator const_iterator;

    const_iterator begin() const { return m_set.begin(); }
    const_iterator end() const { return m_set.end(); }
};

//=======================================================================================
// Arguments for drawing or picking tiles.
// @bsiclass                                                    Keith.Bentley   01/17
//=======================================================================================
struct TileArgs
{
    Transform m_location;
    RootR m_root;
    ClipVectorCP m_clip;

    TileArgs(TransformCR location, RootR root, ClipVectorCP clip) : m_location(location), m_root(root), m_clip(clip) {}
    TransformCR GetLocation() const {return m_location;}
    DPoint3d GetTileCenter(TileCR tile) const {return DPoint3d::FromProduct(GetLocation(), tile.GetCenter());}
    double GetTileRadius(TileCR tile) const {DRange3d range=tile.GetRange(); m_location.Multiply(&range.low, 2); return 0.5 * range.low.Distance(range.high);}
    void SetClip(ClipVectorCP clip) {m_clip = clip;}
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
struct DrawArgs : TileArgs
{
    RenderContextR m_context;
    Render::GraphicBranch m_graphics;
    MissingNodes m_missing;
    BeTimePoint m_now;
    BeTimePoint m_purgeOlderThan;

    DrawArgs(RenderContextR context, TransformCR location, RootR root, BeTimePoint now, BeTimePoint purgeOlderThan, ClipVectorCP clip = nullptr) 
            : TileArgs(location, root, clip), m_context(context), m_now(now), m_purgeOlderThan(purgeOlderThan) {}

    void DrawBranch(Render::ViewFlagsOverridesCR ovrFlags, Render::GraphicBranch& branch);
    void SetClip(ClipVectorCP clip) {m_clip = clip;}
    void Clear() {m_graphics.Clear(); m_missing.clear(); }
    DGNPLATFORM_EXPORT void DrawGraphics(); // place all entries into a GraphicBranch and send it to the ViewContext.

    DGNPLATFORM_EXPORT void RequestMissingTiles(RootR);
    DGNPLATFORM_EXPORT void InsertMissing(TileCR tile);

    double ComputeTileDistance(TileCR tile) const;
    double GetTileSizeModifier() const;
    };
    
//=======================================================================================
//! Arguments for performing a Pick of a tile.   
// @bsiclass                                                    Keith.Bentley   05/16
//=======================================================================================
struct PickArgs : TileArgs
{
    PickContextR m_context;
    PickArgs(PickContextR context, TransformCR location, RootR root, ClipVectorCP clip = nullptr) : TileArgs(location, root, clip), m_context(context) {}
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
    ClipVectorPtr m_clip;      //! clip volume applied to tiles, in root coordinates

    ClipVectorCP _GetClipVector() const override { return m_clip.get(); }

    uint32_t GetMaxPixelSize() const {return m_maxPixelSize;}
    Root(DgnDbR, TransformCR location, Utf8CP rootResource, Render::SystemP system, uint8_t maxZoom, uint32_t maxSize, double transparency=0.0);
};
    
//=======================================================================================
//! A QuadTree tile. May or may not have its graphics present.
// @bsiclass                                                    Keith.Bentley   05/16
//=======================================================================================
struct Tile : TileTree::Tile
{
    DEFINE_T_SUPER(TileTree::Tile)

    bool m_isLeaf;
    TileId m_id; 
    Render::GraphicBuilder::TileCorners m_corners; 
    mutable Render::GraphicPtr m_graphic;                   

    Tile(Root& quadRoot, TileId id, Tile const* parent) : T_Super(quadRoot, parent), m_id(id) {m_isLeaf = (id.m_level == quadRoot.m_maxZoom);}

    TileId      GetTileId() const     {return m_id;}

    uint8_t     GetZoomLevel() const  { return m_id.m_level; }
    uint32_t    GetRow() const        { return m_id.m_row; }
    uint32_t    GetColumn() const     { return m_id.m_column; }

    virtual TilePtr _CreateChild(TileId) const = 0;
    bool _HasChildren() const override {return !m_isLeaf;}
    bool _HasGraphics() const override {return IsReady() && m_graphic.IsValid();}
    void SetIsLeaf() {m_isLeaf = true; /*m_children.clear();*/}
    ChildTiles const* _GetChildren(bool load) const override;
    void _DrawGraphics(TileTree::DrawArgsR) const override;

    Utf8String _GetTileCacheKey () const override {return Utf8PrintfString("%d/%d/%d", m_id.m_level, m_id.m_column, m_id.m_row);}
    Root& GetQuadRoot() const {return (Root&) m_root;}
    double _GetMaximumSize() const override {return GetQuadRoot().GetMaxPixelSize();}
    void _Invalidate() override { m_graphic = nullptr; }
};

} // end QuadTree

//=======================================================================================
//! An OctTree is a 3d TileTree that subdivides each tile into 8 child tiles. The subdivision
//! of a parent tile into its children is not necessarily equal.
//! A tile in an OctTree can be addressed by a TileId comprised of a depth level and 3 indices.
//! The root tile is {0,0,0,0}.
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
namespace OctTree
{
//=======================================================================================
//! Identifies a tile in an OctTree
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct TileId
{
    uint8_t     m_level;
    uint32_t    m_i;
    uint32_t    m_j;
    uint32_t    m_k;

    TileId(uint8_t level, uint32_t i, uint32_t j, uint32_t k) : m_level(level), m_i(i), m_j(j), m_k(k) { }
    TileId() : TileId(0,0,0,0) { }

    TileId CreateChildId(uint32_t i, uint32_t j, uint32_t k) const { return TileId(m_level+1, m_i*2+i, m_j*2+j, m_k*2+k); }
    TileId GetRelativeId(TileId parentId) const;

    static TileId RootId() { return TileId(0,0,0,0); }
};

//=======================================================================================
//! The root of an OctTree
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Root : TileTree::Root
{
    DEFINE_T_SUPER(TileTree::Root);

    DGNPLATFORM_EXPORT Root(DgnDbR db, TransformCR location, Utf8CP rootUrl, Render::SystemP system);
};

//=======================================================================================
//! An OctTree tile. May or may not have its graphics present.
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Tile : TileTree::Tile
{
    DEFINE_T_SUPER(TileTree::Tile);
protected:
    bool                m_isLeaf;
    TileId              m_id;
    Render::GraphicPtr  m_graphic;

    Tile(Root& octRoot, TileId id, Tile const* parent, bool isLeaf) : T_Super(octRoot, parent), m_id(id), m_isLeaf(isLeaf) { }
    DGNPLATFORM_EXPORT DRange3d ComputeChildRange(OctTree::Tile& child) const;

public:
    virtual TileTree::TilePtr _CreateChild(TileId) const = 0;
    bool _HasChildren() const override { return !m_isLeaf; }
    DGNPLATFORM_EXPORT ChildTiles const* _GetChildren(bool load) const override;
    DGNPLATFORM_EXPORT void _DrawGraphics(TileTree::DrawArgsR) const override;
    Utf8String _GetTileCacheKey() const override { return Utf8PrintfString("%d/%d/%d/%d", m_id.m_level, m_id.m_i, m_id.m_j, m_id.m_k); }
    
    TileId GetTileId() const { return m_id; }
    TileId GetRelativeTileId() const;
    bool _HasGraphics() const override { return IsReady() && m_graphic.IsValid(); }
    Root& GetOctRoot() const { return static_cast<Root&>(m_root); }
    Tile const* GetOctParent() const { return static_cast<Tile const*>(GetParent()); }
    Render::GraphicPtr GetGraphic() const { return m_graphic; }
    bool IsLeaf() const { return m_isLeaf; }

    void SetIsLeaf() { m_isLeaf = true; /*m_children.clear();*/ }
    void SetGraphic(Render::GraphicR graphic) { m_graphic = &graphic; }
};

} // end OctTree
END_TILETREE_NAMESPACE

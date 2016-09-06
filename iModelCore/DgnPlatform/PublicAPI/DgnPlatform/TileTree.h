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

#define BEGIN_TILETREE_NAMESPACE     BEGIN_BENTLEY_DGN_NAMESPACE namespace TileTree {
#define END_TILETREE_NAMESPACE       } END_BENTLEY_DGN_NAMESPACE
#define USING_NAMESPACE_TILETREE     using namespace BentleyApi::Dgn::TileTree;

BEGIN_TILETREE_NAMESPACE

/**  @addtogroup GROUP_TileTree TileTree Module

 A TileTree is an abstract class to manage a HLOD (hierarchical level of detail) tree of spatially coherent "3d tiles".

It manages:
 - loading (locally and via http)
 - caching (in memory and on disk)
 - displaying
 - incremental asynchronous refinement

The tree is represented in memory by two classes: TileTree::Root and TileTree::Tile. The tree is in a local
coordinate system, and is positioned in BIM world space by a linear transform.

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
 radius of a sphere enclosing the Tile's range by the size (in meters) of the pixel a the center of the Tile. If the pixel size is
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
 grow beyond the "maxSize" argument passed to TileTree::Root::CreateCache. Tiles are always loaded via the Tile::_LoadTile
 method regardless of whether their data comes from a local file, an HTTP request, or from the local cache.

*/

DEFINE_POINTER_SUFFIX_TYPEDEFS(DrawArgs)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Tile)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Root)

DEFINE_REF_COUNTED_PTR(Tile)
DEFINE_REF_COUNTED_PTR(Root)

typedef std::chrono::steady_clock::time_point TimePoint;

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
//! A Tile in a TileTree. Every Tile has 0 or 1 parent Tile and 0 or more child Tiles. 
//! The range member is an ElementAlignedBox in the local coordinate system of the TileTree.
//! All child Tiles must be contained within the range of their parent Tile.
// @bsiclass                                                    Keith.Bentley   09/16
//=======================================================================================
struct Tile : RefCountedBase, NonCopyableClass
{
    friend struct Root;
    enum LoadState {NotLoaded=0, Queued=1, Loading=2, Ready=3, NotFound=4, Abandoned=5};
    typedef bvector<TilePtr> ChildTiles;

protected:
    mutable ElementAlignedBox3d m_range;
    TileCP m_parent;
    mutable BeAtomic<int> m_loadState;
    mutable ChildTiles m_children;
    mutable TimePoint m_childrenLastUsed; //! automatically updated whenever this tile is used for display

    void SetAbandoned() const;

public:
    Tile(TileCP parent) : m_parent(parent), m_loadState(LoadState::NotLoaded) {}
    DGNPLATFORM_EXPORT void ExtendRange(DRange3dCR childRange) const;
    double GetRadius() const {return 0.5 * m_range.low.Distance(m_range.high);}
    DPoint3d GetCenter() const {return DPoint3d::FromInterpolate(m_range.low, .5, m_range.high);}
    ElementAlignedBox3d GetRange() const {return m_range;}
    DGNPLATFORM_EXPORT void Draw(DrawArgsR, int depth) const;
    LoadState GetLoadState() const {return (LoadState) m_loadState.load();}
    void SetIsReady() {return m_loadState.store(LoadState::Ready);}
    void SetIsQueued() const {return m_loadState.store(LoadState::Queued);}
    void SetNotFound() {return m_loadState.store(LoadState::NotFound);}
    bool IsQueued() const {return m_loadState.load() == LoadState::Queued;}
    bool IsAbandoned() const {return m_loadState.load() == LoadState::Abandoned;}
    bool IsReady() const {return m_loadState.load() == LoadState::Ready;}
    bool IsNotLoaded() const {return m_loadState.load() == LoadState::NotLoaded;}
    bool IsNotFound() const {return m_loadState.load() == LoadState::NotFound;}
    bool IsDisplayable() const {return _GetMaximumSize() > 0.0;}
    TileCP GetParent() const {return m_parent;}
    DGNPLATFORM_EXPORT int CountTiles() const;
    DGNPLATFORM_EXPORT ElementAlignedBox3d ComputeRange() const;

    virtual void _OnChildrenUnloaded() const {}
    DGNPLATFORM_EXPORT virtual void _UnloadChildren(TimePoint olderThan) const;

    //! Load this tile from data supplied. This method is called regardless of the source of the data.
    virtual BentleyStatus _LoadTile(StreamBuffer&, RootR) = 0;

    //! Determine whether this tile has any child tiles. Return true even if the children are not yet created.
    virtual bool _HasChildren() const = 0;

    //! Get the array of children for this Tile.
    //! @param[in] create If false, return nullptr if this tile has children but they are not yet created. 
    //!                   Otherwise create them now.
    virtual ChildTiles const* _GetChildren(bool create) const = 0;

    //! Draw the Graphics of this Tile into args.
    //! @param[in] args The DrawArgs for the current display request.
    //! @param[in] depth The depth of this tile in the tree. This is necessary to sort missing tiles depth-first.
    virtual void _DrawGraphics(DrawArgsR args, int depth) const = 0;

    //! Get the name of this Tile.
    virtual Utf8String _GetTileName() const = 0;

    //! Get the maximum size, in pixels, that this Tile should occupy on the screen. If larger, use its children, if possible.
    virtual double _GetMaximumSize() const = 0;
};

/*=================================================================================**//**
//! The root of a tree of tiles. This object stores the location of the tree relative to the BIM. It also facilitates
//! local caching of HTTP-based tiles.
// @bsiclass                                                    Keith.Bentley   03/16
+===============+===============+===============+===============+===============+======*/
struct Root : RefCountedBase, NonCopyableClass
{
protected:
    bool m_isHttp = false;
    bool m_locatable = false;
    DgnDbR m_db;
    BeFileName m_localCacheName;
    Transform m_location;
    TilePtr m_rootTile;
    Utf8String m_rootUrl;
    Utf8String m_rootDir;
    std::chrono::seconds m_expirationTime = std::chrono::seconds(20); // save unused tiles for 20 seconds
    Dgn::Render::SystemP m_renderSystem = nullptr;
    RealityData::CachePtr m_cache;

public:
    DGNPLATFORM_EXPORT virtual folly::Future<BentleyStatus> _RequestFile(Utf8StringCR, StreamBuffer&);
    DGNPLATFORM_EXPORT virtual folly::Future<BentleyStatus> _RequestTile(TileCR);

    bool IsHttp() const {return m_isHttp;}
    bool IsLocatable() const {return m_locatable;}
    void SetLocatable(bool locatable) {m_locatable = locatable;}
    TransformCR GetLocation() const {return m_location;}
    RealityData::CachePtr GetCache() const {return m_cache;}
    TilePtr GetRootTile() const {return m_rootTile;} //!< Get the root Tile of this Root
    DgnDbR GetDgnDb() const {return m_db;} //!< Get the DgnDb from which this Root was created.
    Dgn::Render::SystemP GetRenderSystem() {return m_renderSystem;}
    ElementAlignedBox3d ComputeRange() const {return m_rootTile->ComputeRange();}

    //! Get the full name of a Tile in this TileTree. By default it concatenates the tile name to the rootDir
    virtual Utf8String _ConstructTileName(TileCR tile) {return m_rootDir + tile._GetTileName();}

    //! Ctor for Root.
    //! @param db The DgnDb from which this Root was created. This is needed to get the Units().GetDgnGCS()'
    //! @param location The transform from tile coordinates to BIM world coordinates.
    //! @param realityCacheName The name of the reality cache database file, relative to the temporary directory.
    //! @param rootUrl The root url for loading tiles. This name will be prepended to tile names.
    //! @param system The Rendering system used to create Render::Graphic used to draw this TileTree.
    DGNPLATFORM_EXPORT Root(DgnDbR db, TransformCR location, Utf8CP realityCacheName, Utf8CP rootUrl, Dgn::Render::SystemP system);

    //! Set expiration time for unused Tiles. During calls to Draw, unused tiles that haven't been used for this number of seconds will be purged.
    void SetExpirationTime(std::chrono::seconds val) {m_expirationTime = val;}

    //! Get expiration time for unused Tiles, in seconds.
    std::chrono::seconds GetExpirationTime() const {return m_expirationTime;}

    //! Create a RealityData::Cache for Tiles from this Root. This will either create or open the SQLite file holding locally cached previously-downloaded versions of Tiles.
    //! @note If this is not an HTTP root, this does nothing.
    DGNPLATFORM_EXPORT void CreateCache(uint64_t maxSize);

    //! Delete, if present, the local SQLite file holding the cache of downloaded tiles.
    //! @note This will first delete the local RealityData::Cache, aborting all outstanding requests and waiting for the queue to empty.
    DGNPLATFORM_EXPORT BentleyStatus DeleteCacheFile();

    //! Traverse the tree and draw the appropriate set of tiles that intersect the view frustum.
    //! @note during the traversal, previously loaded but now unused tiles are purged if they are expired.
    //! @note This method must be called from the client thread
    void Draw(DrawArgs& args) {m_rootTile->Draw(args, 0);}
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

    DPoint3d GetTileCenter(TileCR tile) const {return DPoint3d::FromProduct(m_location, tile.GetCenter());}
    double GetTileRadius(TileCR tile) const {return m_scale * tile.GetRadius();}
    DrawArgs(RenderContextR context, TransformCR location, TimePoint now, TimePoint purgeOlderThan) : m_context(context), m_location(location), m_now(now), m_purgeOlderThan(purgeOlderThan) {m_scale = location.ColumnXMagnitude();}
    DGNPLATFORM_EXPORT void DrawGraphics(ViewContextR); // place all entries into a GraphicBranch and send it to the ViewContext.
    DGNPLATFORM_EXPORT void RequestMissingTiles(RootR);
};

END_TILETREE_NAMESPACE


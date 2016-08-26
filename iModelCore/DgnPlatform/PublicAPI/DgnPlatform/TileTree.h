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

USING_NAMESPACE_BENTLEY_DGN

DEFINE_POINTER_SUFFIX_TYPEDEFS(DrawArgs)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Tile)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Root)

DEFINE_REF_COUNTED_PTR(Tile)
DEFINE_REF_COUNTED_PTR(Root)

typedef std::chrono::steady_clock::time_point TimePoint;

//=======================================================================================
// A ByteStream with a "current position". Used for reading tiles
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct Tile : RefCountedBase, NonCopyableClass
{
    friend struct Root;
    enum class VisitComplete {Yes=1, No=0,};
    enum LoadState {NotLoaded=0, Queued=1, Loading=2, Ready=3, NotFound=4, Abandoned=5};
    typedef bvector<TilePtr> ChildTiles;

protected:
    mutable ElementAlignedBox3d m_range;
    DPoint3d m_center;
    double m_radius = 0.0;
    double m_maxSize = 0.0;
    TileP m_parent;
    mutable BeAtomic<int> m_loadState;
    mutable ChildTiles m_children;
    mutable TimePoint m_childrenLastUsed;

    void SetAbandoned() const;

public:
    Tile(TileP parent) : m_parent(parent), m_loadState(LoadState::NotLoaded) {m_center.Zero();}
    double GetMaximumSize() const {return m_maxSize;}
    double GetRadius() const {return m_radius;}
    DPoint3dCR GetCenter() const {return m_center;}
    ElementAlignedBox3d GetRange() const {return m_range;}
    DGNPLATFORM_EXPORT VisitComplete Visit(DrawArgsR, int depth) const;
    LoadState GetLoadState() const {return (LoadState) m_loadState.load();}
    void SetIsReady() {return m_loadState.store(LoadState::Ready);}
    void SetIsQueued() const {return m_loadState.store(LoadState::Queued);}
    void SetNotFound() {return m_loadState.store(LoadState::NotFound);}
    bool IsQueued() const {return m_loadState.load() == LoadState::Queued;}
    bool IsAbandoned() const {return m_loadState.load() == LoadState::Abandoned;}
    bool IsReady() const {return m_loadState.load() == LoadState::Ready;}
    bool IsNotLoaded() const {return m_loadState.load() == LoadState::NotLoaded;}
    bool IsNotFound() const {return m_loadState.load() == LoadState::NotFound;}
    bool IsDisplayable() const {return m_maxSize > 0.0;}
    TileCP GetParent() const {return m_parent;}
    DGNPLATFORM_EXPORT int CountTiles() const;
    DGNPLATFORM_EXPORT ElementAlignedBox3d ComputeRange() const;

    DGNPLATFORM_EXPORT virtual void _UnloadChildren(TimePoint olderThan) const;

    virtual BentleyStatus _Read(StreamBuffer&, RootR) = 0;
    virtual bool _HasChildren() const = 0;
    virtual ChildTiles const* _GetChildren() const = 0;
    virtual VisitComplete _Draw(DrawArgsR, int depth) const = 0;
    virtual Utf8String _GetTileName() const = 0;
};

/*=================================================================================**//**
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
    RealityData::CachePtr m_cache;

public:
    bool IsHttp() const {return m_isHttp;}
    void Draw(DrawArgs& args) {m_rootTile->Visit(args, 0);}
    void SetExpirationTime(std::chrono::seconds val) {m_expirationTime = val;} //! set expiration time for unused nodes
    std::chrono::seconds GetExpirationTime() const {return m_expirationTime;} //! get expiration time for unused nodes
    bool IsLocatable() const {return m_locatable;}
    void SetLocatable(bool locatable) {m_locatable = locatable;}
    TransformCR GetLocation() const {return m_location;}
    RealityData::CachePtr GetCache() const {return m_cache;}
    TilePtr GetRoot() {return m_rootTile;}
    ElementAlignedBox3d ComputeRange() const {return m_rootTile->ComputeRange();}
    DGNPLATFORM_EXPORT void CreateCache();
    DGNPLATFORM_EXPORT BentleyStatus DeleteCacheFile(); //! delete the local SQLite file holding the cache of downloaded tiles.
    DGNPLATFORM_EXPORT Root(DgnDbR, TransformCR location, Utf8CP realityCacheName, Utf8CP rootUrl);

    DGNPLATFORM_EXPORT virtual folly::Future<BentleyStatus> _RequestFile(Utf8StringCR, StreamBuffer&);
    DGNPLATFORM_EXPORT virtual folly::Future<BentleyStatus> _RequestTile(TileCR);
    virtual Utf8String _ConstructTileName(TileCR tile) {return m_rootDir + tile._GetTileName();}
};

//=======================================================================================
// Arguments for drawing a node. As nodes are drawn, their Render::Graphics go into the GraphicArray member of this object. After all
// in-view nodes are drawn, the accumulated list of Render::Graphics are placed in a Render::GroupNode with the "location"
// transform for the scene (that is, the tile graphics are always in the local coordinate system of the 3mx scene.)
// If higher resolution tiles are needed but missing, the graphics for lower resolution tiles are
// drawn and the missing tiles are requested for download (if necessary.) They are then added to the MissingNodes member. If the
// MissingNodes list is not empty, we schedule a ProgressiveDisplay that checks for the arrival of the missing nodes and draws them (using
// this class). Each iteration of ProgressiveDisplay starts with a list of previously-missing tiles and generates a new list of
// still-missing tiles until all have arrived (or the view changes.)
// @bsiclass                                                    Keith.Bentley   05/16
//=======================================================================================
struct DrawArgs
{
    typedef bmultimap<int, TileCPtr> MissingNodes;
    RenderContextR m_context;
    Transform m_location;
    double m_scale;
    Render::GraphicBranch m_graphics;
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


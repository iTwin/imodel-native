/*--------------------------------------------------------------------------------------+ 
|
|     $Source: PublicAPI/DgnPlatform/TileTree.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnPlatform.h"
#include <set>
#include <forward_list>
#include <Bentley/CancellationToken.h>
#include <DgnPlatform/RealityDataCache.h>
#include <forward_list>

#define BEGIN_TILETREE_NAMESPACE    BEGIN_BENTLEY_DGN_NAMESPACE namespace TileTree {
#define END_TILETREE_NAMESPACE      } END_BENTLEY_DGN_NAMESPACE
#define USING_NAMESPACE_TILETREE    using namespace BentleyApi::Dgn::TileTree;

BEGIN_TILETREE_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(Tile)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Root)
DEFINE_POINTER_SUFFIX_TYPEDEFS(TileLoader)
DEFINE_POINTER_SUFFIX_TYPEDEFS(LoadContext)
DEFINE_POINTER_SUFFIX_TYPEDEFS(TileCache)
DEFINE_POINTER_SUFFIX_TYPEDEFS(StreamBuffer)
DEFINE_POINTER_SUFFIX_TYPEDEFS(TileMetadata)
DEFINE_POINTER_SUFFIX_TYPEDEFS(ClassificationPreprocessor)

DEFINE_REF_COUNTED_PTR(Tile)
DEFINE_REF_COUNTED_PTR(Root)
DEFINE_REF_COUNTED_PTR(TileLoader)
DEFINE_REF_COUNTED_PTR(TileCache)
DEFINE_REF_COUNTED_PTR(ClassificationPreprocessor)

//=======================================================================================
// Manage the creation and cleanup of the local TileCache used by TileData
// @bsiclass                                                    Keith.Bentley   08/16
//=======================================================================================
struct TileCache : RealityData::Cache
{
    uint64_t m_allowedSize;
private:

    TileCache(uint64_t maxSize) : m_allowedSize(maxSize) {}

    bool ValidateData() const;
    bool WriteCurrentVersion() const;
public:
    BentleyStatus _Prepare() const final;
    BentleyStatus _Initialize() const final;
    BentleyStatus _Cleanup() const final;

    static BeFileName GetCacheFileName(BeFileNameCR baseName);

    static BeSQLite::PropertySpec GetVersionSpec() { return BeSQLite::PropertySpec("binaryFormatVersion", "elementTileCache"); }
    static Utf8CP GetCurrentVersion();

    static RealityData::CachePtr Create(DgnDbCR db);
};

typedef std::shared_ptr<struct TileLoadState> TileLoadStateSPtr;

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
    void ResetPos() {SetPos(0);}
    uint32_t GetPos() const {return m_currPos;}
    DGNPLATFORM_EXPORT bool ReadBytes(void* buf, uint32_t size);
    template<typename T> bool Read (T& buf) { return ReadBytes(&buf, sizeof(buf)); }

    bool Skip(uint32_t numBytes)
        {
        auto newPos = m_currPos + numBytes;
        if (newPos > GetSize())
            return false;

        SetPos(newPos);
        return true;
        }

    StreamBuffer() {}
    StreamBuffer(ByteStream const& other) : ByteStream(other) {}
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   09/16
//=======================================================================================
struct TileLoadState : ICancellationToken, NonCopyableClass
{
private:
    TileCPtr        m_tile;
    BeAtomic<bool>  m_canceled;
public:
    explicit TileLoadState(TileCR tile) : m_tile(&tile) { }
    DGNPLATFORM_EXPORT ~TileLoadState();
    bool IsCanceled() override {return m_canceled.load();}
    void SetCanceled() {m_canceled.store(true);}
    void Register(std::weak_ptr<ICancellationListener> listener) override {}
    TileCR GetTile() const { return *m_tile; }

    struct PtrComparator
    {
        using is_transparent = std::true_type;

        bool operator()(TileLoadStateSPtr const& lhs, TileLoadStateSPtr const& rhs) const { return operator()(lhs->m_tile.get(), rhs->m_tile.get()); }
        bool operator()(TileLoadStateSPtr const& lhs, TileCP rhs) const { return operator()(lhs->m_tile.get(), rhs); }
        bool operator()(TileCP lhs, TileLoadStateSPtr const& rhs) const { return operator()(lhs, rhs->m_tile.get()); }
        bool operator()(TileCP lhs, TileCP rhs) const { return lhs < rhs; }
    };
};

//=======================================================================================
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

    bool operator==(TileId const& other) const { return m_level == other.m_level && m_i == other.m_i && m_j == other.m_j && m_k == other.m_k; }
    bool IsRoot() const { return *this == RootId(); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/18
//=======================================================================================
enum class TileFlags : uint32_t
{
    None            = 0,
    ContainsCurves  = 0x0001 << 0, // This tile's geometry includes curves.
    Incomplete      = 0x0001 << 1, // This tile's range contains some geometry too small to contribute to tile's geometry.
    IsLeaf          = 0x0001 << 2, // This tile has no children.
    IsDisplayable = 0x0001 << 3, // This tile has geometry. It may have children with geometry.
    HasZoomFactor   = 0x0001 << 4, // This tile does not subdivide but does refine into a single higher-resolution child tile.
};

ENUM_IS_FLAGS(TileFlags);

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/18
//=======================================================================================
struct TileMetadata
{
private:
    TileFlags           m_flags = TileFlags::None;
    ElementAlignedBox3d m_contentRange;
    double              m_zoomFactor = 1.0;
public:
    Json::Value ToJson() const;
    void FromJson(Json::Value const& json);

    void Reset() { *this = TileMetadata(); }

    TileFlags GetFlags() const { return m_flags; }
    bool IsFlagSet(TileFlags flag) const { return flag == (m_flags & flag); }
    void SetFlag(TileFlags flag, bool set) { m_flags = set ? (m_flags | flag) : (m_flags & ~flag); }

    bool IsLeaf() const { return IsFlagSet(TileFlags::IsLeaf); }
    void SetIsLeaf(bool isLeaf) { SetFlag(TileFlags::IsLeaf, isLeaf); }

    bool IsDisplayable() const { return IsFlagSet(TileFlags::IsDisplayable); }
    void SetIsDisplayable(bool displayable) { SetFlag(TileFlags::IsDisplayable, displayable); }

    void SetContainsCurves(bool contains) { SetFlag(TileFlags::ContainsCurves, contains); }
    void SetIsIncomplete(bool incomplete) { SetFlag(TileFlags::Incomplete, incomplete); }

    bool HasZoomFactor() const { return IsFlagSet(TileFlags::HasZoomFactor); }
    double GetZoomFactor() const { return m_zoomFactor; }
    void SetZoomFactor(double zoomFactor) { SetFlag(TileFlags::HasZoomFactor, true); m_zoomFactor = zoomFactor; }
    void ClearZoomFactor() { SetFlag(TileFlags::HasZoomFactor, false); m_zoomFactor = 1.0; }

    ElementAlignedBox3dCR GetContentRange() const { return m_contentRange; }
    void SetContentRange(ElementAlignedBox3dCR range) { m_contentRange = range; }
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

private:
    RootR   m_root;
    mutable ElementAlignedBox3d m_range;
    TileCP  m_parent;
    uint32_t m_depth;
    mutable BeAtomic<LoadStatus> m_loadStatus;
    mutable ChildTiles m_children;
    TileId m_id;
    TileMetadata m_metadata;
    double m_tolerance;

    Tile(RootR root, TileId id, TileCP parent, bool isLeaf);
    Tile(Root& root, TileTree::TileId id, Tile const* parent, DRange3dCP range, bool displayable);
    explicit Tile(Tile const& parent);
    Tile(Root& root, TileTree::TileId id, DRange3dCR range, double minToleranceRatio);

    void SetAbandoned() const;

    void InitTolerance(double minToleranceRatio, bool isLeaf=false);
    DGNPLATFORM_EXPORT DRange3d ComputeChildRange(Tile& child, bool is2d=false) const;
public:
    ElementAlignedBox3d const& GetRange() const {return m_range;}
    DGNPLATFORM_EXPORT ElementAlignedBox3d ComputeRange() const;
    DGNPLATFORM_EXPORT void ExtendRange(DRange3dCR childRange) const;

    TileId GetTileId() const { return m_id; }
    TileId GetRelativeTileId() const;

    double GetRadius() const {return 0.5 * m_range.low.Distance(m_range.high);}
    uint32_t GetDepth() const {return m_depth;}
    DPoint3d GetCenter() const {return DPoint3d::FromInterpolate(m_range.low, .5, m_range.high);}

    LoadStatus GetLoadStatus() const {return (LoadStatus) m_loadStatus.load();}
    bool IsQueued() const {return m_loadStatus.load() == LoadStatus::Queued;}
    bool IsAbandoned() const {return m_loadStatus.load() == LoadStatus::Abandoned;}
    bool IsReady() const {return m_loadStatus.load() == LoadStatus::Ready;}
    bool IsNotLoaded() const {return m_loadStatus.load() == LoadStatus::NotLoaded;}
    bool IsNotFound() const {return m_loadStatus.load() == LoadStatus::NotFound;}
    void SetIsReady() {return m_loadStatus.store(LoadStatus::Ready);}
    void SetIsQueued() const {return m_loadStatus.store(LoadStatus::Queued);}
    void SetNotLoaded() {return m_loadStatus.store(LoadStatus::NotLoaded);}
    void SetNotFound() {return m_loadStatus.store(LoadStatus::NotFound);}

    TileMetadataCR GetMetadata() const { return m_metadata; }
    TileMetadataR GetMetadata() { return m_metadata; }
    void SetMetadata(TileMetadataCR metadata) { m_metadata = metadata; }

    void SetZoomFactor(double zoom) { BeAssert(!IsLeaf()); m_metadata.SetZoomFactor(zoom); }
    bool HasZoomFactor() const { return m_metadata.HasZoomFactor(); }
    double GetZoomFactor() const { return HasZoomFactor() ? m_metadata.GetZoomFactor() : 1.0; }

    bool IsDisplayable() const { return m_metadata.IsDisplayable(); }
    bool IsParentDisplayable() const {return nullptr != GetParent() && GetParent()->IsDisplayable();}
    void SetDisplayable(bool displayable) { m_metadata.SetIsDisplayable(displayable); }

    TileCP GetParent() const {return m_parent;}
    RootCR GetRoot() const {return m_root;}
    RootR GetRoot() {return m_root;}
    DGNPLATFORM_EXPORT Render::SystemR GetRenderSystem() const;

    DGNPLATFORM_EXPORT void UnloadChildren(BeTimePoint olderThan) const;

    //! Determine whether this tile has any child tiles. Return true even if the children are not yet created.
    bool HasChildren() const { return !IsLeaf(); }
    bool IsLeaf() const { return m_metadata.IsLeaf(); }
    void SetIsLeaf(bool isLeaf = true) { m_metadata.SetIsLeaf(isLeaf); }

    //! Get the array of children for this Tile.
    //! @param[in] create If false, return nullptr if this tile has children but they are not yet created. Otherwise create them now.
    DGNPLATFORM_EXPORT ChildTiles const* GetChildren(bool create) const;
    TilePtr CreateChild(TileId) const;

    //! Called when tile data is required.
    TileLoaderPtr CreateTileLoader(TileLoadStateSPtr);

    //! Get the tile cache key for this Tile.
    DGNPLATFORM_EXPORT Utf8String GetTileCacheKey() const;

    //! Get the maximum size, in pixels, that this Tile should occupy on the screen. If larger, use its children, if possible.
    double GetMaximumSize() const;

    //! Returns a potentially more tight-fitting range enclosing the visible contents of this tile.
    ElementAlignedBox3d const& GetContentRange() const { return HasContentRange() ? m_metadata.GetContentRange() : m_range; }
    bool HasContentRange() const { return !m_metadata.GetContentRange().IsNull(); }
    void SetContentRange(ElementAlignedBox3dCR contentRange) { m_metadata.SetContentRange(contentRange); }

    bool IsEmpty() const;

    DGNPLATFORM_EXPORT bool ToJson(Json::Value&) const;

    double GetTolerance() const { return m_tolerance; }
    DRange3d GetDgnRange() const;
    DRange3d GetTileRange() const { return GetRange(); }

    Utf8String GetIdString() const;
    TilePtr FindTile(TileTree::TileId id, double zoomFactor);
    TilePtr FindTile(double zoomFactor);

    static TilePtr Create(Root& root, TileTree::TileId id, Tile const& parent) { return new Tile(root, id, &parent, nullptr, true); }
    static TilePtr CreateRoot(Root& root, DRange3dCR range, bool populate) { return new Tile(root, TileTree::TileId::RootId(), nullptr, &range, populate); }
    static TilePtr CreateWithZoomFactor(Tile const& parent) { return new Tile(parent); }
};

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     08/2018
//=======================================================================================
struct ClassificationPreprocessor : RefCountedBase
{
    double      m_offset;
    DRange3d    m_range;

    ClassificationPreprocessor(Root const& tile);
    void Preprocess(CurveVectorPtr& curveVector);
    void Preprocess(PolyfaceHeaderPtr& polyface);
    bool SeperatePrimitivesById() const { return true; }

};

/*=================================================================================**//**
//! The root of a tree of tiles. This object stores the location of the tree relative to world coordinates. 
// @bsiclass                                                    Keith.Bentley   03/16
+===============+===============+===============+===============+===============+======*/
struct Root : RefCountedBase, NonCopyableClass
{
    friend TileLoader;

private:
    mutable std::mutex m_dbMutex;
    mutable BeConditionVariable m_cv;
    mutable std::set<TileLoadStateSPtr, TileLoadState::PtrComparator> m_activeLoads;
    DgnDbR m_db;
    DgnModelId m_modelId;
    Transform m_location;         // transform from tile coordinates to world coordinates
    TilePtr m_rootTile;
    Dgn::Render::SystemR m_renderSystem;
    RealityData::CachePtr m_cache;
    bool m_is3d;
    ClassificationPreprocessorPtr m_classificationPreprocessor;

    //! Clear the current tiles and wait for all pending download requests to complete/abort.
    //! All subclasses of Root must call this method in their destructor. This is necessary, since it must be called while the subclass vtable is 
    //! still valid and that cannot be accomplished in the destructor of Root.
    DGNPLATFORM_EXPORT void ClearAllTiles(); 

    Root(GeometricModelCR model, TransformCR location, Dgn::Render::SystemR system, bool asClassifier);

    bool LoadRootTile(DRange3dCR range, GeometricModelR model, bool populate);
public:
    DGNPLATFORM_EXPORT BentleyStatus RequestTile(TileR tile);

    ~Root() { ClearAllTiles(); }

    void StartTileLoad(TileLoadStateSPtr) const;
    void DoneTileLoad(TileLoadStateSPtr) const;

    DGNPLATFORM_EXPORT void CancelTileLoad(TileCR tile);
    DGNPLATFORM_EXPORT void CancelAllTileLoads();
    void WaitForAllLoads() {BeMutexHolder holder(m_cv.GetMutex()); while (m_activeLoads.size()>0) m_cv.InfiniteWait(holder);}
    DGNPLATFORM_EXPORT void WaitForAllLoadsFor(uint32_t milliseconds);

    TransformCR GetLocation() const {return m_location;}
    void SetLocation(TransformCR location) {m_location = location;}

    RealityData::CachePtr GetCache() const {return m_cache;}

    TilePtr GetRootTile() const {return m_rootTile;} //!< Get the root Tile of this Root
    DgnDbR GetDgnDb() const {return m_db;} //!< Get the DgnDb from which this Root was created.
    DgnModelId GetModelId() const{return m_modelId;} //!< Get the ID of the GeometricModel from which this Root was created.
    ElementAlignedBox3d ComputeRange() const {return m_rootTile->ComputeRange();}
    Dgn::Render::SystemR GetRenderSystem() const {return m_renderSystem;}

    //! Get the resource name (file name or URL) of a Tile in this TileTree. By default it concatenates the tile cache key to the rootResource
    Utf8String ConstructTileResource(TileCR tile) const {return tile.GetTileCacheKey();}

    bool Is3d() const { return m_is3d; }
    bool Is2d() const { return !Is3d(); }

    DGNPLATFORM_EXPORT ByteStream GetTileDataFromCache(Utf8StringCR cacheKey) const;

    GeometricModelPtr GetModel() const { return GetDgnDb().Models().Get<GeometricModel>(GetModelId()); }
    std::mutex& GetDbMutex() const { return m_dbMutex; }

    template<typename T> auto UnderMutex(T func) const -> decltype(func()) { BeMutexHolder lock(m_cv.GetMutex()); return func(); }

    bool IsClassifier() const { return m_classificationPreprocessor.IsValid(); } //!< @private
    ClassificationPreprocessor* GetPreprocessor() const { return m_classificationPreprocessor.get(); } //!< @private

    DGNPLATFORM_EXPORT bool ToJson(Json::Value&) const;
    DGNPLATFORM_EXPORT TilePtr FindTileById(Utf8CP id);

    DGNPLATFORM_EXPORT static RootPtr Create(GeometricModelR model, Render::SystemR system, bool asClassifier);
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
    TilePtr                     m_tile;             // tile to load, cannot be null.
    TileLoadStateSPtr           m_loads;
    BeSQLite::SnappyFromBlob    m_snappyFrom;
    BeSQLite::SnappyToBlob      m_snappyTo;

    // Cacheable information
    Utf8String m_cacheKey;      // for loading or saving to tile cache
    StreamBuffer m_tileBytes;   // when available, bytes are saved here
    TileMetadata m_tileMetadata;

    uint64_t m_createTime;       // The time of the most recent change to any element in the associated model when the tile loader was created.
    uint64_t m_cacheCreateTime;  // The time of the most recent change to any element in the associated model when the tile cache data was created.
    bool m_saveToCache = false;

    TileLoader(TileR tile, TileLoadStateSPtr loads);

    BentleyStatus LoadTile();
    BentleyStatus DoReadFromDb();
    BentleyStatus DoSaveToDb();
    BentleyStatus DropFromDb(RealityData::CacheR);

    uint64_t GetCreateTime() const { return m_createTime; }
public:
    bool IsCanceledOrAbandoned() const {return (m_loads != nullptr && m_loads->IsCanceled()) || m_tile->IsAbandoned();}
    Dgn::Render::SystemR GetRenderSystem() const { return m_tile->GetRenderSystem(); }

    DGNPLATFORM_EXPORT BentleyStatus SaveToDb();
    DGNPLATFORM_EXPORT BentleyStatus ReadFromDb();

    //! Called to get the data from the original location. The call must be fast and execute any long running operation asynchronously.
    virtual BentleyStatus _GetFromSource() = 0;

    //! Load tile. This method is called when the tile data becomes available, regardless of the source of the data. Called from worker threads.
    virtual BentleyStatus _LoadTile() = 0; 

    //! Given the time (in unix millis) at which the tile was written to the cache, return true if the cached tile is no longer usable.
    //! If so, it will be deleted from the cache and _LoadTile() will be used to produce a new tile instead.
    bool IsExpired(uint64_t cachedTileCreateTime);

    //! Return false if m_tileBytes contains invalid data and should be updated from source.
    bool IsValidData();

    struct LoadFlag
        {
        TileLoadStateSPtr m_state;
        LoadFlag(TileLoadStateSPtr state) : m_state(state) {state->GetTile().GetRoot().StartTileLoad(state);}
        ~LoadFlag() {m_state->GetTile().GetRoot().DoneTileLoad(m_state);}
        };

    //! Perform the load asynchronously.
    BentleyStatus Perform();

    TileR GetTile() { return *m_tile; }
    TileCR GetTile() const { return *m_tile; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct LoadContext
{
private:
    TileLoaderCP    m_loader;
public:
    explicit LoadContext(TileLoaderCP loader) : m_loader(loader) { }

    bool WasAborted() const { return nullptr != m_loader && m_loader->IsCanceledOrAbandoned(); }
    Dgn::Render::SystemR GetRenderSystem() const {return m_loader->GetRenderSystem();}
};

END_TILETREE_NAMESPACE

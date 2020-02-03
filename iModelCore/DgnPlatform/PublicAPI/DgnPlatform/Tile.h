/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeSQLite/BeSQLite.h>
#include "DgnPlatform.h"
#include <DgnPlatform/RealityDataCache.h>
#include <DgnPlatform/RenderPrimitives.h>
#include <set>

#define BEGIN_TILE_NAMESPACE    BEGIN_BENTLEY_DGN_NAMESPACE namespace Tile {
#define END_TILE_NAMESPACE      } END_BENTLEY_DGN_NAMESPACE
#define USING_NAMESPACE_TILE    using namespace BentleyApi::Dgn::Tile;

BEGIN_TILE_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(NodeId);
DEFINE_POINTER_SUFFIX_TYPEDEFS(ContentId);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Cache);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Tree);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Content);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Loader);
DEFINE_POINTER_SUFFIX_TYPEDEFS(AnimationNodeMap);

DEFINE_REF_COUNTED_PTR(Cache);
DEFINE_REF_COUNTED_PTR(Tree);
DEFINE_REF_COUNTED_PTR(Content);
DEFINE_REF_COUNTED_PTR(Loader);

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/18
//=======================================================================================
struct Cache : RealityData::Cache
{
private:
    uint64_t m_allowedSize;

    Cache(uint64_t maxSize) : m_allowedSize(maxSize) {}

    bool ValidateData() const;
    bool WriteCurrentVersion() const;
public:
    BentleyStatus _Prepare() const final;
    BentleyStatus _Initialize() const final;
    BentleyStatus _Cleanup() const final;

    static BeFileName GetCacheFileName(BeFileNameCR baseName);

    static BeSQLite::PropertySpec GetVersionSpec() { return BeSQLite::PropertySpec("binaryFormatVersion", "elementTileCache"); }
    static Utf8String GetCurrentVersion();

    static RealityData::CachePtr Create(DgnDbCR db);
};

//=======================================================================================
//! Uniquely identifies a volume within the oct-tree by its depth and range.
// @bsistruct                                                   Paul.Connelly   08/18
//=======================================================================================
struct NodeId
{
protected:
    uint64_t    m_i;
    uint64_t    m_j;
    uint64_t    m_k;
    uint8_t     m_depth;

    static int8_t Compare(NodeIdCR lhs, NodeIdCR rhs)
        {
        if (lhs.m_depth != rhs.m_depth) return lhs.m_depth < rhs.m_depth ? -1 : 1;
        else if (lhs.m_i != rhs.m_i) return lhs.m_i < rhs.m_i ? -1 : 1;
        else if (lhs.m_j != rhs.m_j) return lhs.m_j < rhs.m_j ? -1 : 1;
        else if (lhs.m_k != rhs.m_k) return lhs.m_k < rhs.m_k ? -1 : 1;
        else return 0;
        }

    void EnforceDepth()
        {
        if (0 == m_depth)
            {
            BeAssert(0 == m_i && 0 == m_j && 0 == m_k);
            m_i = m_j = m_k = 0;
            }
        }
public:
    NodeId(uint8_t depth, uint64_t i, uint64_t j, uint64_t k) : m_i(i), m_j(j), m_k(k), m_depth(depth) { EnforceDepth(); }
    NodeId() : m_i(0), m_j(0), m_k(0), m_depth(0) { }

    static NodeId RootId() { return NodeId(); }
    bool IsRoot() const { return 0 == GetDepth(); }

    uint8_t GetDepth() const { return m_depth; }

    bool operator==(NodeIdCR other) const { return 0 == Compare(*this, other); }
    bool operator!=(NodeIdCR other) const { return !(*this == other); }
    bool operator<(NodeIdCR rhs) const { return Compare(*this, rhs) < 0; }

    DGNPLATFORM_EXPORT NodeId ComputeParentId() const;
    NodeId ComputeChildId(bool i, bool j, bool k) { return NodeId(m_depth + 1, m_i * 2 + (i ? 1 : 0), m_j * 2 + (j ? 1 : 0), m_k * 2 + (k ? 1 : 0)); }
    DGNPLATFORM_EXPORT DRange3d ComputeRange(DRange3dCR rootRange, bool is2d) const;
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/18
//=======================================================================================
struct ContentId : NodeId
{
    enum class Flags : uint32_t
    {
        None = 0,
        AllowInstancing = 1 << 0,
        ImprovedElision = 1 << 1,
    };
private:
    DEFINE_T_SUPER(NodeId);

    uint32_t    m_mult;
    Flags       m_flags;
    uint16_t    m_majorVersion;

    Utf8String Format(uint64_t const* parts, size_t numParts) const;
    bool FromV1String(Utf8CP);
    bool FromV4String(Utf8CP, uint16_t majorVersion);
public:
    ContentId(NodeIdCR nodeId, uint32_t mult, uint16_t version, Flags flags) : T_Super(nodeId), m_mult(0 == mult ? 1 : mult), m_majorVersion(version), m_flags(flags) { }
    ContentId(uint8_t depth, uint64_t i, uint64_t j, uint64_t k, uint32_t mult, uint16_t version, Flags flags) : ContentId(NodeId(depth, i, j, k), mult, version, flags) { }
    ContentId() : ContentId(0, 0, 0, 0, 1, 1, Flags::None) { }

    bool operator==(ContentIdCR other) const { return T_Super::operator==(other) && m_mult == other.m_mult && m_majorVersion == other.m_majorVersion && m_flags == other.m_flags; }
    bool operator!=(ContentIdCR other) const { return !(*this == other); }
    bool operator<(ContentIdCR rhs) const
        {
        auto compId = Compare(*this, rhs);
        if (0 != compId) return compId < 0;
        else if (m_mult != rhs.m_mult) return m_mult < rhs.m_mult;
        else if (m_majorVersion != rhs.m_majorVersion) return m_majorVersion < rhs.m_majorVersion;
        else if (m_flags != rhs.m_flags) return m_flags < rhs.m_flags;
        else return false;
        }

    DGNPLATFORM_EXPORT Utf8String ToString() const;
    DGNPLATFORM_EXPORT bool FromString(Utf8CP str, uint16_t version);

    double GetSizeMultiplier() const { BeAssert(0 != m_mult); return static_cast<double>(m_mult); }
    uint16_t GetMajorVersion() const { BeAssert(0 < m_majorVersion); return m_majorVersion; }
    Flags GetFlags() const { return m_flags; }

    DGNPLATFORM_EXPORT bool IsFlagSet(Flags flag) const;
    bool AllowInstancing() const { return IsFlagSet(Flags::AllowInstancing); }
    bool WantImprovedElision() const { return IsFlagSet(Flags::ImprovedElision); }
};

ENUM_IS_FLAGS(ContentId::Flags);

//=======================================================================================
//! Representation of geometry contained within a tile.
// @bsistruct                                                   Paul.Connelly   08/18
//=======================================================================================
struct Content : RefCountedBase
{
    //=======================================================================================
    // @bsistruct                                                   Paul.Connelly   08/18
    //=======================================================================================
    enum class Flags : uint32_t
    {
        None = 0,
        ContainsCurves = 1 << 0, // This tile's content includes curved geometry.
        Incomplete = 1 << 2, // Some geometry was excluded from this tile's content.
    };

    // Bitfield wherein each bit corresponds to a sub-volume of a tile's volume.
    enum class SubRange : uint32_t
    {
        None = 0,
        k000 = 1 << 0,
        k100 = 1 << 1,
        k010 = 1 << 2,
        k110 = 1 << 3,
        k001 = 1 << 4,
        k101 = 1 << 5,
        k011 = 1 << 6,
        k111 = 1<< 7,
    };

    //=======================================================================================
    //! Describes aspects of tile Content. Used by front-end to make decisions about tile
    //! subdivision, among other things. The metadata is encoded into the header within the
    //! binary stream.
    // @bsistruct                                                   Paul.Connelly   08/18
    //=======================================================================================
    struct Metadata
    {
        ElementAlignedBox3d m_contentRange; // A sub-range of the tile's range tightly enclosing its geometry.
        double m_tolerance = 0.0; // Chord tolerance at which geometry was facetted.
        uint32_t m_numElementsIncluded = 0; // Number of elements whose geometry (or a subset thereof) is included in this content.
        uint32_t m_numElementsExcluded = 0; // Number of elements within the tile's range which contributed no geometry to this content.
        Flags m_flags = Flags::None;
        SubRange m_emptySubRanges = SubRange::None; // Bitfield wherein a 1 bit indicates an empty sub-volume.
    };

    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(Metadata);
private:
    ByteStream m_bytes;
public:
    explicit Content(ByteStream&& bytes) : m_bytes(std::move(bytes)) { }

    ByteStreamCR GetBytes() const { return m_bytes; }
};

ENUM_IS_FLAGS(Content::Flags);
ENUM_IS_FLAGS(Content::SubRange);

//=======================================================================================
//! Loads tile content from cache, or generates it from geometry and adds to cache.
// @bsistruct                                                   Paul.Connelly   08/18
//=======================================================================================
struct Loader : RefCountedBase, NonCopyableClass
{
    enum class State : uint8_t
    {
        Loading,
        Ready,
        NotFound,
        Invalid,
    };

    friend struct Tree;
private:
    ContentId m_contentId;
    ContentPtr m_content;
    TreeR m_tree;
    Utf8String m_cacheKey;
    BeAtomic<bool> m_canceled;
    BeSQLite::BeGuid m_geometryGuid;
    State m_state;
    bool m_useCache;

    void SetState(State state) { m_state = state; }
    void SetCanceled() { m_canceled.store(true); }
    void SetReady() { SetState(State::Ready); }
    void SetNotFound() { SetState(State::NotFound); }
    void SetInvalid() { SetState(State::Invalid); }

    BeSQLite::BeGuid GetGeometryGuid() const { return m_geometryGuid; }
    bool IsExpired(BeSQLite::BeGuid geometryGuid) const;
    bool IsValidData(ByteStreamCR bytes) const;
    BentleyStatus DropFromDb(RealityData::CacheR db);
    BentleyStatus SaveToDb();

    State ReadFromCache();
    State ReadFromModel();
    void Perform();
protected:
    DGNPLATFORM_EXPORT Loader(TreeR tree, ContentIdCR contentId, bool useCache);
public:
    DGNPLATFORM_EXPORT ~Loader();

    bool IsCanceled() const { return m_canceled.load(); }
    State GetState() const { return m_state; }
    bool IsLoading() const { return State::Loading == GetState(); }
    bool IsReady() const { return State::Ready == GetState(); }
    bool IsNotFound() const { return State::NotFound == GetState(); }
    bool IsInvalid() const { return State::Invalid == GetState(); }

    ContentIdCR GetContentId() const { return m_contentId; }
    bool AllowInstancing() const;
    bool WantImprovedElision() const { return m_contentId.WantImprovedElision(); }
    ContentCP GetContent() const { return m_content.get(); }
    TreeR GetTree() const { return m_tree; }

    virtual void Preprocess(Render::Primitives::PolyfaceList& polyfaces, Render::Primitives::StrokesList& strokes) const { }
    virtual bool CompressMeshQuantization() const { return false; } // If true the quantization of each mesh will be compressed to include only the mesh (not tile) range.  ...Classifiers;
    virtual uint64_t GetNodeId(DgnElementId, Render::Primitives::DisplayParamsCR) const;

    struct PtrComparator
    {
        using is_transparent = std::true_type;

        bool operator()(LoaderPtr const& lhs, LoaderPtr const& rhs) const { return operator()(lhs->GetContentId(), rhs->GetContentId()); }
        bool operator()(LoaderPtr const& lhs, ContentIdCR rhs) const { return operator()(lhs->GetContentId(), rhs); }
        bool operator()(ContentIdCR lhs, LoaderPtr const& rhs) const { return operator()(lhs, rhs->GetContentId()); }
        bool operator()(ContentIdCR lhs, ContentIdCR rhs) const { return lhs < rhs; }
    };

    struct IdentityComparator
    {
        bool operator()(LoaderPtr const& lhs, LoaderPtr const& rhs) const { return lhs.get() < rhs.get(); }
    };
};

//=======================================================================================
// For a TileTree generated with a schedule script, maps element IDs to the animation
// nodes containing them.
// Node IDs (referred to as "batch IDs" in the schedule script json) begin at 1 - an
// ID of 0 does not refer to any node.
// Nodes which have cutting planes or transforms applied to them cannot be batched with
// geometry belonging to other nodes. Nodes which have only symbology/visibility
// overrides applied to them can be batched together.
// @bsistruct                                                   Paul.Connelly   01/19
//=======================================================================================
struct AnimationNodeMap
{
private:
    uint32_t                    m_maxNodeIndex = 0;
    bmap<uint64_t, uint32_t>    m_elemIdToNodeIndex;
    bset<uint32_t>              m_discreteNodeIndices;
public:
    void Populate(DisplayStyleCR style, DgnModelId modelId);

    bool IsEmpty() const { return m_elemIdToNodeIndex.empty(); }
    uint32_t GetMaxNodeIndex() const { return m_maxNodeIndex; }
    uint32_t GetNodeIndex(uint64_t elemId) const;
    uint32_t GetDiscreteNodeIndex(uint64_t elemId) const;
    uint32_t GetNodeIndex(DgnElementId elemId) const { return GetNodeIndex(elemId.GetValueUnchecked()); }
    void Clear() { m_maxNodeIndex = 0; m_elemIdToNodeIndex.clear(); m_discreteNodeIndices.clear(); }
};

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     12/18
//=======================================================================================
struct NodeMap : RefCountedBase, bmap<uint64_t, uint64_t>  {};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/18
//=======================================================================================
struct Tree : RefCountedBase, NonCopyableClass
{
    enum class Type : uint8_t
    {
        Model,
        VolumeClassifier,
        PlanarClassifier,
    };

    enum class RootTile : uint8_t
    {
        Empty,
        Undisplayable,
        Displayable,
    };

    enum class Flags : uint32_t
    {
        None = 0,
        UseProjectExtents = 1 << 0, //!< Use project extents as basis for tile tree volume.
        EnforceDisplayPriority = 1 << 1, //!< For 3d plan projection models, group graphics into "layers" by subcategory.
    };

    struct Id
    {
    private:
        DgnModelId      m_modelId;
        DgnElementId    m_animationSourceId;
        double          m_expansion;
        Tree::Flags     m_flags;
        uint16_t        m_majorVersion;
        Tree::Type      m_type;
        bool            m_omitEdges;

        DGNPLATFORM_EXPORT static uint16_t GetDefaultMajorVersion();
        Id(DgnModelId modelId, Tree::Type type, Tree::Flags flags, uint16_t majorVersion, bool omitEdges, double expansion, DgnElementId animId)
            : m_modelId(modelId), m_animationSourceId(animId), m_expansion(expansion), m_flags(flags), m_majorVersion(majorVersion), m_type(type), m_omitEdges(omitEdges) { }

        void Invalidate() { *this = Id(); }
    public:
        Id() : Id(DgnModelId(), Tree::Type::Model, Tree::Flags::None, GetDefaultMajorVersion(), true, 0.0, DgnElementId()) { }

        // Create an Id for a classifier tree
        Id(DgnModelId modelId, Tree::Flags flags, uint16_t majorVersion, double expansion, bool isPlanar, DgnElementId animationId = DgnElementId())
            : Id(modelId, isPlanar ? Tree::Type::PlanarClassifier : Tree::Type::VolumeClassifier, flags, majorVersion, true, expansion, animationId) { }

        // Create an Id for a V3 classifier tree
        Id(DgnModelId modelId, double expansion, bool isPlanar) : Id(modelId, isPlanar ? Tree::Flags::None : Tree::Flags::UseProjectExtents, 3, expansion, isPlanar, DgnElementId()) { }

        // Create an Id for a model or animation tree
        Id(DgnModelId modelId, Tree::Flags flags, uint16_t majorVersion, bool omitEdges, DgnElementId animationSourceId=DgnElementId())
            : Id(modelId, Tree::Type::Model, flags, majorVersion, omitEdges, 0.0, animationSourceId) { }

        // Create an Id for a V3 model or animation tree
        Id(DgnModelId modelId, bool omitEdges, DgnElementId animationSourceId=DgnElementId()) : Id(modelId, Tree::Flags::None, 3, omitEdges, animationSourceId) { }

        DGNPLATFORM_EXPORT bool IsValid() const;
        DGNPLATFORM_EXPORT bool IsValidContentId(ContentIdCR) const;

        Tree::Type GetType() const { return m_type; }
        bool ContainsAnimation() const { return m_animationSourceId.IsValid(); }
        bool IsPlanarClassifier() const { return Tree::Type::PlanarClassifier == GetType(); }
        bool IsVolumeClassifier() const { return Tree::Type::VolumeClassifier == GetType(); }
        bool IsClassifier() const { return IsPlanarClassifier() || IsVolumeClassifier(); }

        DgnModelId GetModelId() const { return m_modelId; }
        uint16_t GetMajorVersion() const { return m_majorVersion; }
        bool GetOmitEdges() const { return m_omitEdges; }
        double GetClassifierExpansion() const { return m_expansion; }
        DgnElementId GetAnimationSourceId() const { return m_animationSourceId; }

        Tree::Flags GetFlags() const { return m_flags; }
        DGNPLATFORM_EXPORT bool IsFlagSet(Tree::Flags flag) const;
        bool GetUseProjectExtents() const { return IsFlagSet(Tree::Flags::UseProjectExtents); }
        bool GetEnforceDisplayPriority() const { return IsFlagSet(Tree::Flags::EnforceDisplayPriority); }

        // If a DgnDb is supplied, the Id is validated against its contents. e.g., a model Id pointing to a non-existent or non-geometric model is invalid.
        // Otherwise only basic validation of the Id format itself is performed.
        // Returns an invalid Id if validation fails.
        DGNPLATFORM_EXPORT static Id FromString(Utf8StringCR idString, DgnDbP db);
        DGNPLATFORM_EXPORT Utf8String ToString() const;
        DGNPLATFORM_EXPORT Utf8String GetPrefixString() const;
    };

private:
    friend struct LoaderScope;
    struct LoaderScope
    {
        LoaderPtr m_loader;
        BeMutexHolder& m_lock;

        LoaderScope(LoaderR loader, BeMutexHolder& lock);
        ~LoaderScope();
    };

    mutable std::mutex m_dbMutex;
    mutable BeConditionVariable m_cv;
    DgnDbR m_db;
    Transform m_location;         // transform from tile coordinates to world coordinates
    ElementAlignedBox3d m_range;
    Render::SystemR m_renderSystem;
    RealityData::CachePtr m_cache;
    std::set<LoaderPtr, Loader::PtrComparator> m_activeLoads;
    bset<LoaderPtr, Loader::IdentityComparator> m_canceledLoads;
    Id m_id;
    bool m_is3d;
    RootTile m_rootTile;
    AnimationNodeMap m_nodeMap;
    DRange3d m_contentRange;
    Utf8String m_contentIdQualifier;

    void CancelTileLoad(LoaderPtr const& loader);
protected:
    Tree(GeometricModelCR model, TransformCR location, DRange3dCR range, Render::SystemR system, Id id, RootTile rootTile, DRange3dCR contentRange);

    DGNPLATFORM_EXPORT LoaderPtr CreateLoader(ContentIdCR contentId, bool useCache);
    void LoadNodeMapFromAnimation();
public:
    DGNPLATFORM_EXPORT ~Tree();
    DGNPLATFORM_EXPORT void Destroy();

    DgnDbR GetDgnDb() const { return m_db; }
    std::mutex& GetDbMutex() { return m_dbMutex; }
    DgnModelId GetModelId() const { return GetId().GetModelId(); }
    Type GetType() const { return GetId().GetType(); }
    bool IsVolumeClassifier() const { return GetId().IsVolumeClassifier(); }
    bool IsPlanarClassifier() const { return GetId().IsPlanarClassifier(); }
    bool GetOmitEdges() const { return GetId().GetOmitEdges(); }
    ElementAlignedBox3dCR GetRange() const { return m_range; }
    Render::SystemR GetRenderSystem() const { return m_renderSystem; }
    TransformCR GetLocation() const { return m_location; }
    RealityData::CacheP GetCache() const { return m_cache.get(); }
    uint64_t GetDiscreteNodeId(DgnElementId elementId) const;
    AnimationNodeMapCR GetAnimationNodeMap() const { return m_nodeMap; }

    // Instancing not supported for classifiers or animated geometry.
    bool AllowInstancing() const { return Type::Model == GetType(); }

    bool Is3d() const { return m_is3d; }
    bool Is2d() const { return !Is3d(); }

    DGNPLATFORM_EXPORT Json::Value ToJson() const;
    GeometricModelPtr FetchModel() const { return GetDgnDb().Models().Get<GeometricModel>(GetModelId()); }
    Utf8String ConstructCacheKey(ContentIdCR contentId) const { auto key = GetId().ToString(); key.append(contentId.ToString()); return key; }

    DGNPLATFORM_EXPORT void CancelTileLoads(bvector<ContentId> const& contentIds);
    DGNPLATFORM_EXPORT void CancelAllTileLoads();
    DGNPLATFORM_EXPORT void WaitForAllLoads();

    // Obtain the content associated with the specified content ID.
    // If another thread is already in the process of obtaining the same content, this thread will wait until the other thread completes, then return the same content.
    // Otherwise, this thread will synchronously obtain the content.
    // If the content is available in the RealityData::Cache, it will be retrieved from there.
    // Otherwise, it will be generated from the geometry within the content range, added to the cache, and returned.
    // If the content cannot be retrieved, or the loading process is canceled, returns nullptr.
    DGNPLATFORM_EXPORT ContentCPtr RequestContent(ContentIdCR contentId, bool useCache);

    Id GetId() const { return m_id; }

    DGNPLATFORM_EXPORT static TreePtr Create(GeometricModelR model, Render::SystemR system, Id id);
    DGNPLATFORM_EXPORT static Type TypeFromId(Utf8StringCR treeId);
    DGNPLATFORM_EXPORT static Type ExtractTypeFromid(Utf8StringR treeId);

    bool IsValidContentId(ContentIdCR contentId) const { return GetId().IsValidContentId(contentId); }
};

ENUM_IS_FLAGS(Tree::Flags);

END_TILE_NAMESPACE


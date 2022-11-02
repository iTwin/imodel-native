/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Geom/BRepEntity.h>
#include <Bentley/Nullable.h>

//=======================================================================================
//! NOTE: Most of the types in this header are implementation details, exposed strictly
//! for testing.
// @bsistruct
//=======================================================================================
BEGIN_BENTLEY_RENDER_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(BRepMetadata);
DEFINE_POINTER_SUFFIX_TYPEDEFS(IBRepDataSupplier);
DEFINE_POINTER_SUFFIX_TYPEDEFS(BRepHandleId);
DEFINE_POINTER_SUFFIX_TYPEDEFS(BRepHandle);
DEFINE_POINTER_SUFFIX_TYPEDEFS(BRepHandleStorage);
DEFINE_POINTER_SUFFIX_TYPEDEFS(IBRepService);
DEFINE_POINTER_SUFFIX_TYPEDEFS(BRepFacetRequest);
DEFINE_POINTER_SUFFIX_TYPEDEFS(BRepFacets);
DEFINE_POINTER_SUFFIX_TYPEDEFS(BRepFacetRequestQueue);
DEFINE_POINTER_SUFFIX_TYPEDEFS(IBRepInfoCache);

DEFINE_REF_COUNTED_PTR(BRepFacetRequest);
DEFINE_REF_COUNTED_PTR(BRepHandle);
DEFINE_REF_COUNTED_PTR(IBRepService);

using IBRepInfoCacheUPtr = std::unique_ptr<IBRepInfoCache>;

//=======================================================================================
// @bsistruct
//=======================================================================================
enum class BRepFlags : uint8_t {
    None = 0,
    Sheet = static_cast<uint8_t>(IBRepEntity::EntityType::Sheet),
    Wire = static_cast<uint8_t>(IBRepEntity::EntityType::Wire),
    Curved = 1 << 2,
    FaceAttachments = 1 << 3,
};

ENUM_IS_FLAGS(BRepFlags);

static_assert(static_cast<uint8_t>(BRepFlags::Sheet) == 1 << 0, "enum mismatch");
static_assert(static_cast<uint8_t>(BRepFlags::Wire) == 1 << 1, "enum mismatch");

//=======================================================================================
//! Metadata describing an IBRepEntity.
// @bsistruct
//=======================================================================================
struct BRepMetadata {
    using EntityType = IBRepEntity::EntityType;
private:
    DRange3d m_range;
    EntityType m_type;
    bool m_hasCurvedFaceOrEdge;
    bool m_hasFaceAttachments;

    static EntityType EntityTypeFromFlags(BRepFlags flags)
        {
        return static_cast<EntityType>(flags & (BRepFlags::Sheet | BRepFlags::Wire));
        }
public:
    BRepMetadata() : BRepMetadata(DRange3d::NullRange(), false, false, EntityType::Invalid) { }
    BRepMetadata(DRange3dCR range, bool hasCurvedFaceOrEdge, bool hasFaceAttachments, EntityType type)
        : m_range(range), m_type(type), m_hasCurvedFaceOrEdge(hasCurvedFaceOrEdge), m_hasFaceAttachments(hasFaceAttachments) { }
    BRepMetadata(DRange3dCR range, BRepFlags flags)
        : BRepMetadata(range, BRepFlags::None != (flags & BRepFlags::Curved), BRepFlags::None != (flags & BRepFlags::FaceAttachments), EntityTypeFromFlags(flags)) { }
    explicit BRepMetadata(IBRepEntityCR entity)
        : BRepMetadata(entity.GetEntityRange(), entity.HasCurvedFaceOrEdge(), nullptr != entity.GetFaceMaterialAttachments(), entity.GetEntityType()) { }

    DRange3dCR GetRange() const { return m_range; }
    bool HasCurvedFaceOrEdge() const { return m_hasCurvedFaceOrEdge; }
    bool HasFaceAttachments() const { return m_hasFaceAttachments; }
    EntityType GetType() const { return m_type; }

    BRepFlags GetFlags() const {
        auto flags = static_cast<BRepFlags>(GetType());
        if (HasCurvedFaceOrEdge())
            flags |= BRepFlags::Curved;

        if (HasFaceAttachments())
            flags |= BRepFlags::FaceAttachments;

        return flags;
    }
};

//=======================================================================================
//! Facets (and optional face attachments) produced by a BRepHandle for an IBRepEntity.
// @bsistruct
//=======================================================================================
struct BRepFacets {
    bvector<PolyfaceHeaderPtr> m_polyfaces;
    bvector<FaceAttachment> m_faceAttachments;

    bool IsEmpty() const { return m_polyfaces.empty(); }
    void Clear() { m_polyfaces.clear(); m_faceAttachments.clear(); }

    BRepFacets Clone() const
        {
        BRepFacets clone;
        clone.m_faceAttachments = m_faceAttachments;
        clone.m_polyfaces.reserve(m_polyfaces.size());
        for (auto& polyface : m_polyfaces)
            clone.m_polyfaces.push_back(polyface->Clone());

        return clone;
        }
};

//=======================================================================================
//! An object that can supply the binary flatbuffer representation of an IBRepEntity.
// @bsistruct
//=======================================================================================
struct IBRepDataSupplier {
    struct Data {
        uint8_t const* m_bytes;
        uint32_t m_size;
    };

    virtual ~IBRepDataSupplier() { }

    // Supply the flatbuffer representation. The pointer in the return value remains valid only for as long as thie IBRepDataSupplier does.
    virtual Data SupplyData() const = 0;
};

//=======================================================================================
//! Wraps a callable object that returns BRep data.
// @bsistruct
//=======================================================================================
template<typename T> struct BRepDataSupplier : IBRepDataSupplier {
protected:
    T m_supplyData;
public:
    explicit BRepDataSupplier(T supplyData) : m_supplyData(supplyData) { }

    Data SupplyData() const override { return m_supplyData(); }
};

//=======================================================================================
//! Stable cache identifier for a BRep in a geometry stream.
//! It refers either to a geometry part (Id and index) or an element (Id and index plus
//! GUIDs).
// @bsistruct
//=======================================================================================
struct BRepHandleId {
private:
    BeSQLite::BeGuid m_geometryGuid;
    BeSQLite::BeGuid m_sessionGuid;
    uint64_t m_entityId;
    uint16_t m_geometryIndex;
public:
    BRepHandleId() : m_entityId(0), m_geometryIndex(0) { }

    BRepHandleId(DgnElementId partId, uint16_t geometryIndex) : m_entityId(partId.GetValueUnchecked()), m_geometryIndex(geometryIndex) { }

    BRepHandleId(DgnElementId elementId, uint16_t geometryIndex, BeSQLite::BeGuid geomGuid, BeSQLite::BeGuid sessionGuid) :
        m_entityId(elementId.GetValueUnchecked()), m_geometryIndex(geometryIndex), m_geometryGuid(geomGuid), m_sessionGuid(sessionGuid) { }

    static BRepHandleId FromGeometryStreamEntryId(DgnElementId geometricElementId, GeometryStreamEntryIdCR entryId, BeSQLite::BeGuid geomGuid, BeSQLite::BeGuid sessionGuid) {
        BeAssert(entryId.IsValid());

        // Per Brien, part geometry is assumed not to change. Parts don't reside in geometric models anyway, so GeometryGuid is irrelevant anyway.
        auto partId = entryId.GetGeometryPartId();
        if (partId.IsValid())
            return BRepHandleId(partId, entryId.GetPartIndex());

        return BRepHandleId(geometricElementId, entryId.GetIndex(), geomGuid, sessionGuid);
    }

    uint64_t GetEntityId() const { return m_entityId; }
    uint16_t GetGeometryIndex() const { return m_geometryIndex; }
    BeSQLite::BeGuid GetGeometryGuid() const { return m_geometryGuid; }
    BeSQLite::BeGuid GetSessionGuid() const { return m_sessionGuid; }
    bool IsValid() const { return 0 != m_entityId; }

    bool operator<(BRepHandleIdCR rhs) const {
        if (m_entityId != rhs.m_entityId)
            return m_entityId < rhs.m_entityId;
        if (m_geometryIndex != rhs.m_geometryIndex)
            return m_geometryIndex < rhs.m_geometryIndex;
        if (m_geometryGuid != rhs.m_geometryGuid)
            return m_geometryGuid < rhs.m_geometryGuid;

        return m_sessionGuid < rhs.m_sessionGuid;
    }

    bool operator==(BRepHandleIdCR rhs) const {
        return m_entityId == rhs.m_entityId && m_geometryIndex == rhs.m_geometryIndex
            && m_geometryGuid == rhs.m_geometryGuid && m_sessionGuid == rhs.m_sessionGuid;
    }
};

//=======================================================================================
//! Interface adopted by an object that can produce BRepHandles, possibly from a cache;
//! and provide metadata and facets for them, possibly from a cache or a BRepFacetRequest.
// @bsistruct
//=======================================================================================
struct IBRepService : RefCountedBase {
    virtual ~IBRepService() { }
    virtual BRepHandlePtr GetBRepHandle(BRepHandleIdCR id, IBRepDataSupplierCR, ICancellableP) = 0;
    virtual BRepFacets ProduceFacets(BRepHandleR, IFacetOptionsR, ICancellableP) = 0;
    virtual void OnLastReferenceToHandle() { }
};

//=======================================================================================
//! A BRepHandle obtained from (and possibly cached by) an IBRepService.
//! Under the hood it may contain either an IBRepEntityPtr or the binary flatbuffer data
//! required to instantiate the entity later.
// @bsistruct
//=======================================================================================
struct BRepHandle : IRefCounted {
protected:
    const BRepMetadata m_metadata;
    const BRepHandleId m_id;
    IBRepServiceR m_service;
    // The mutable members are accessed exclusively by ObtainEntity which is invoked exclusively by BRepFacetRequest on the facetter thread.
    mutable bvector<uint8_t> m_flatBufferData;
    mutable IBRepEntityCPtr m_entity;
    mutable BeAtomic<uint32_t> m_refCount;

    BRepHandle(BRepHandleIdCR id, BRepMetadataCR metadata, IBRepServiceR service) : m_id(id), m_metadata(metadata), m_service(service) { }
public:
    DEFINE_BENTLEY_NEW_DELETE_OPERATORS                 \

    BRepHandle(BRepHandleIdCR id, BRepMetadataCR metadata, bvector<uint8_t>&& flatBufferData, IBRepServiceR service) : BRepHandle(id, metadata, service) {
        m_flatBufferData = std::move(flatBufferData);
    }

    BRepHandle(BRepHandleIdCR id, BRepMetadataCR metadata, IBRepDataSupplier::Data const& data, IBRepServiceR service) : BRepHandle(id, metadata, service) {
        m_flatBufferData = bvector<uint8_t>(data.m_bytes, data.m_bytes + data.m_size);
    }

    BRepHandle(BRepHandleId id, IBRepEntityCR entity, IBRepServiceR service) : BRepHandle(id, BRepMetadata(entity), service) {
        m_entity = &entity;
    }

    BRepHandleId GetId() const { return m_id; }
    BRepMetadataCR GetMetadata() const { return m_metadata; }
    DRange3dCR GetEntityRange() const { return GetMetadata().GetRange(); }
    bool HasCurvedFaceOrEdge() const { return GetMetadata().HasCurvedFaceOrEdge(); }
    bool HasFaceAttachments() const { return GetMetadata().HasFaceAttachments(); }
    IBRepEntity::EntityType GetEntityType() const { return GetMetadata().GetType(); }

    // Strictly to be accessed by IBRepService with protection against race conditions.
    DGNPLATFORM_EXPORT IBRepEntityCPtr ObtainEntity() const;

    BRepFacets ProduceFacets(IFacetOptionsR facetOptions, ICancellableP cancel) {
        return m_service.ProduceFacets(*this, facetOptions, cancel);
    }

    uint32_t AddRef() const final {
        return m_refCount.IncrementAtomicPre();
    }

    uint32_t Release() const final {
        auto& service = m_service;
        uint32_t countWas = m_refCount.DecrementAtomicPost();
        REFCOUNT_RELEASE_CHECK(countWas);
        if (1 == countWas)
            delete this;
        else if (2 == countWas)
            service.OnLastReferenceToHandle();

        return countWas - 1;
    }

    uint32_t GetRefCount() const {
        return m_refCount.load();
    }

    struct PtrComparator {
        using is_transparent = std::true_type;

        bool operator()(BRepHandlePtr const& lhs, BRepHandlePtr const& rhs) const { return lhs->GetId() < rhs->GetId(); }
        bool operator()(BRepHandlePtr const& lhs, BRepHandleIdCR rhs) const { return lhs->GetId() < rhs; }
        bool operator()(BRepHandleIdCR lhs, BRepHandlePtr const& rhs) const { return lhs < rhs->GetId(); }
    };
};

//=======================================================================================
//! Tracks extant BRepHandles by Id. Handles are eligible for removal when the last
//! strong reference to them goes out of scope. This is used to avoid having multiple
//! handles to the same entity; otherwise each would have an identical copy of the
//! IBRepEntity or the flat buffer data required to instantiate it.
// @bsistruct
//=======================================================================================
struct BRepHandleStorage {
protected:
    BeMutex m_mutex;
    bset<BRepHandlePtr, BRepHandle::PtrComparator> m_handles;
    BeAtomic<bool> m_haveGarbage;

    BRepHandlePtr _Find(BRepHandleIdCR id, bool collectGarbage) {
        if (collectGarbage)
            collectGarbage = m_haveGarbage.exchange(false);

        if (!collectGarbage) {
            auto found = m_handles.find(id);
            return m_handles.end() == found ? nullptr : *found;
        }

        BRepHandlePtr found;
        auto iter = m_handles.begin();
        auto end = m_handles.end();
        while (iter != end) {
            auto const& handle = *iter;
            if (found.IsNull() && handle->GetId() == id) {
                found = handle;
                ++iter;
            } else if (handle->GetRefCount() <= 1) {
                // Garbage - cache holds the last reference.
                BeAssert(handle->GetRefCount() > 0);
                iter = m_handles.erase(iter);
                end = m_handles.end();
            } else {
                ++iter;
            }
        }

        return found;
    }
public:
    BRepHandlePtr Find(BRepHandleIdCR id, bool collectGarbage) {
        BeMutexHolder lock(m_mutex);
        return _Find(id, collectGarbage);
    }

    template<typename T> BRepHandlePtr Obtain(BRepHandleIdCR id, T createFunc) {
        // Try to find it in cache under the mutex.
        auto handle = Find(id, true);
        if (handle.IsValid())
            return handle;

        // Instantiate outside the mutex.
        auto newHandle = createFunc();
        if (newHandle.IsNull())
            return newHandle;

        // Under the mutex, check if someone added it to the cache while we waited; otherwise, add it to the cache ourselves.
        BeMutexHolder lock(m_mutex);
        handle = _Find(id, false);
        if (handle.IsNull())
            m_handles.insert(handle = newHandle);

        return handle;
    }

    void ScheduleGarbageCollection() {
        m_haveGarbage.store(true);
    }
};

struct BRepFacetOptions {
    int32_t m_toleranceLog2;
    bool m_paramsRequired;

    BRepFacetOptions(int32_t toleranceLog2, bool paramsRequired) : m_toleranceLog2(toleranceLog2), m_paramsRequired(paramsRequired) { }
    BRepFacetOptions(IFacetOptionsCR facetOptions, BRepHandleCR handle) :
        m_toleranceLog2(ComputeToleranceLog2(facetOptions.GetChordTolerance(), handle.HasCurvedFaceOrEdge())),
        m_paramsRequired(facetOptions.GetParamsRequired()) { }

    bool operator==(BRepFacetOptions const& rhs) const {
        return m_toleranceLog2 == rhs.m_toleranceLog2 && m_paramsRequired == rhs.m_paramsRequired;
    }

    static constexpr int32_t kUncurvedToleranceLog2 = std::numeric_limits<int32_t>::min();
    static constexpr int32_t ComputeToleranceLog2(double chordTolerance, bool isCurved) {
        BeAssert(chordTolerance > 0.0);
        return isCurved && chordTolerance > 0.0 ? static_cast<int32_t>(floor(log2(chordTolerance))) : kUncurvedToleranceLog2;
    }
};

//=======================================================================================
//! A cache of metadata and facets indexed by BRepHandleId.
// @bsistruct
//=======================================================================================
struct IBRepInfoCache {
public:
    virtual ~IBRepInfoCache() { }
    virtual Nullable<BRepMetadata> LoadMetadata(BRepHandleIdCR) = 0;
    virtual Nullable<BRepFacets> LoadFacets(BRepHandleIdCR, BRepFacetOptions) = 0;
    virtual void StoreMetadata(BRepHandleIdCR, BRepMetadataCR) = 0;
    virtual void StoreFacets(BRepHandleIdCR, BRepFacetOptions, BRepFacetsCR) = 0;
};

//=======================================================================================
//! Represents a request for a BRepFacetRequestQueue to produce facets for a BRep.
//! Multiple requestors can be serviced by the same request.
//! Individual requestors can cancel their interest in the results of the request.
//! Requests are pulled off of a queue.
//! Requests for which all requestors have canceled their interest are not processed.
// @bsistruct
//=======================================================================================
struct BRepFacetRequest : RefCountedBase {
protected:
    BeConditionVariable m_cv;
    bset<ICancellableP> m_requestors;
    BRepHandlePtr m_handle;
    IFacetOptionsPtr m_facetOptions;
    BRepFacetOptions m_brepOptions;
    BRepFacets m_facets;
    BeAtomic<bool> m_finished;

    DGNPLATFORM_EXPORT BRepFacetRequest(BRepHandleR handle, IFacetOptionsCR facetOptions, ICancellableP requestor);

    friend struct Scope;
    struct Scope {
        BRepFacetRequestR m_req;
        explicit Scope(BRepFacetRequestR req) : m_req(req) { }
        ~Scope();
    };
public:
    static BRepFacetRequestPtr Create(BRepHandleR handle, IFacetOptionsCR facetOptions, ICancellableP requestor)
        {
        return new BRepFacetRequest(handle, facetOptions, requestor);
        }

    static double ComputeMinimumChordTolerance(DRange3dCR entityRange)
        {
        constexpr double minRangeRelTol = 1.0e-4;
        double rangeDiagonal = entityRange.DiagonalDistance();
        return rangeDiagonal * minRangeRelTol;
        }

    bool Matches(BRepHandleIdCR id, BRepFacetOptions opts) const
        {
        return m_brepOptions == opts && m_handle->GetId() == id;
        }

    // Iterate all requestors and remove any that have been canceled. Notifies condition variable if any became canceled.
    // Caller is responsible for synchronization.
    DGNPLATFORM_EXPORT void UpdateRequestors();
    // Returns true if all requestors have been canceled. Caller is responsible for synchronization.
    bool IsCanceled() const { return m_requestors.empty(); }
    // Returns true if ProduceFacets() completed.
    bool IsFinished() const { return m_finished.load(); }

    // Add to the set of requestors interested in the results of this request. Caller is responsible for synchronization.
    void AddRequestor(ICancellableP requestor) { m_requestors.insert(requestor); }

    // Condition variable that notifies when any requestor becomes canceled, or when ProduceFacets completes.
    BeConditionVariableR GetConditionVariable() { return m_cv; }

    // Notifies condition variable when complete. Pick up results from GetFacets.
    DGNPLATFORM_EXPORT void ProduceFacets();

    // After ProduceFacets has completed, holds the facets produced from the IBRepEntity.
    // Contents should be treated as immutable.
    BRepFacetsCR GetFacets() const { return m_facets; }
};

//=======================================================================================
// @bsistruct
//=======================================================================================
struct BRepFacetRequestQueue : NonCopyableClass {
    enum class State : uint8_t {
        // Start was not yet called.
        NotStarted,
        // Start was called. Facetter thread is running.
        Running,
        // Set by destructor on any thread except facetter thread to indicate facetter thread should terminate.
        Terminating,
        // Set by facetter thread just before it terminates.
        Terminated,
    };
protected:
    BeConditionVariable m_cv;
    std::deque<BRepFacetRequestPtr> m_queue;
    BRepFacetRequestPtr m_active;
    BeAtomic<State> m_state;

    THREAD_MAIN_DECL Main(void*);
    // Main loop, does not exit until Stop is called. Processes active request.
    void Process();
    // Pulls next request off queue and assigns to m_active.
    void WaitForWork();
    // Removes any requests from the queue that have become canceled.
    void UpdateRequestors();
public:
    BRepFacetRequestQueue() { }
    ~BRepFacetRequestQueue() { Stop(); }

    // Start the queue processor on a separate thread. Must be invoked no more than once.
    DGNPLATFORM_EXPORT void Start();

    // Exposed as public strictly for tests. Invoked by destructor.
    DGNPLATFORM_EXPORT void Stop();

    // Obtain a request from the queue to facet the specified BRep using the specified facet options.
    // If no compatible request exists on the queue, a new one is appended and returned.
    DGNPLATFORM_EXPORT BRepFacetRequestPtr Get(BRepHandleR handle, IFacetOptionsCR facetOptions, ICancellableP requestor);

    State GetState() { return m_state.load(); }
    bool IsRunning() { return State::Running == GetState(); }
    bool IsTerminating() { return State::Terminating == GetState(); }
    bool IsTerminated() { return State::Terminated == GetState(); }

    // The following are strictly for tests.
    std::deque<BRepFacetRequestPtr> const& GetQueue() { return m_queue; }
    BRepFacetRequestPtr GetActive() { BeMutexHolder lock(m_cv.GetMutex()); return m_active; }
    size_t size() const
        {
        BeMutexHolder lock(m_cv.GetMutex());
        return m_queue.size() + (m_active.IsValid() ? 1 : 0);
        }
    void clear()
        {
        BeMutexHolder lock(m_cv.GetMutex());
        m_active = nullptr;
        m_queue.clear();
        }
    void SetActive(BRepFacetRequestR newActive)
        {
        BeMutexHolder lock(m_cv.GetMutex());
        auto oldActive = m_active;
        if (oldActive.IsValid())
            m_queue.push_back(oldActive);

        auto iter = std::find_if(m_queue.begin(), m_queue.end(), [&](BRepFacetRequestPtr const& p) { return p.get() == &newActive; });
        m_queue.erase(iter);
        m_active = &newActive;
        }
};

//=======================================================================================
//! An IBRepService that uses a BRepFacetQueue executing on a dedicated thread to produce
//! facets; and a sqlite database to cache facets and metadata.
//! Must be initialized from the main thread.
// @bsistruct
//=======================================================================================
struct BRepService : IBRepService {
protected:
    BRepFacetRequestQueue m_queue;
    BRepHandleStorage m_handles;
    IBRepInfoCacheUPtr m_cache;
public:
    DGNPLATFORM_EXPORT BRepService(IBRepInfoCacheUPtr&&, bool startThread);

    DGNPLATFORM_EXPORT BRepHandlePtr GetBRepHandle(BRepHandleIdCR, IBRepDataSupplierCR, ICancellableP) override;
    DGNPLATFORM_EXPORT BRepFacets ProduceFacets(BRepHandleR, IFacetOptionsR, ICancellableP) override;
    void OnLastReferenceToHandle() override { m_handles.ScheduleGarbageCollection(); }
};

END_BENTLEY_RENDER_NAMESPACE

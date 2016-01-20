/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/LocksManager.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatform.h>

DGNPLATFORM_TYPEDEFS(LockRequest);
DGNPLATFORM_TYPEDEFS(DgnLock);
DGNPLATFORM_TYPEDEFS(DgnLockOwnership);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct DgnChangeSummary;

//=======================================================================================
//! Enumerates the types of objects that can be locked by a briefcase.
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
enum class LockableType
{
    Db, //!< The DgnDb itself.
    Model, //!< A DgnModel.
    Element, //!< A DgnElement.
};

//=======================================================================================
//! Identifies a lockable object by type and ID.
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct LockableId
{
private:
    BeSQLite::BeInt64Id m_id;
    LockableType m_type;
public:
    LockableId() : m_type(LockableType::Element) { } //!< Constructs an invalid LockableId.
    explicit LockableId(DgnElementId id) : m_id(id), m_type(LockableType::Element) { } //!< Constructs a LockableId for an element
    explicit LockableId(DgnModelId id) : m_id(id), m_type(LockableType::Model) { } //!< Constructs a LockableId for a model
    LockableId(LockableType type, BeSQLite::BeInt64Id id) : m_id(id), m_type(type) { } //!< Constructs a LockableId of the specified type and ID
    DGNPLATFORM_EXPORT explicit LockableId(DgnDbCR db); //!< Constructs a LockableId for a DgnDb
    DGNPLATFORM_EXPORT explicit LockableId(DgnModelCR model); //!< Constructs a LockableId for a model
    DGNPLATFORM_EXPORT explicit LockableId(DgnElementCR element); //!< Constructs a LockableId for an element

    BeSQLite::BeInt64Id GetId() const { return m_id; } //!< The ID of the lockable object
    LockableType GetType() const { return m_type; } //!< The type of the lockable object
    bool IsValid() const { return m_id.IsValid(); } //!< Determine if this LockableId refers to a valid object
    void Invalidate() { m_id.Invalidate(); } //!< Invalidates this LockableId

    //! Compare two LockableIds for sorting purposes
    bool operator<(LockableId const& rhs) const
        {
        return m_type != rhs.m_type ? m_type < rhs.m_type : m_id < rhs.m_id;
        }

    //! Compare two LockableIds for equality
    bool operator==(LockableId const& rhs) const { return m_type == rhs.m_type && m_id == rhs.m_id; }

    //! Compare two LockableIds for inequality
    bool operator!=(LockableId const& rhs) const { return !(*this == rhs); }

    DGNPLATFORM_EXPORT void ToJson(JsonValueR value) const; //!< Convert to JSON representation
    DGNPLATFORM_EXPORT bool FromJson(JsonValueCR value); //!< Attempt to initialize from JSON representation
};

//! A set of identifiers for lockable objects
typedef bset<LockableId> LockableIdSet;

//=======================================================================================
//! Enumerates the possible levels of ownership granted to a briefcase by a lock.
//! Shared locks can be acquired by any number of briefcases, as long as no briefcase holds
//! an exclusive lock on the same object.
//! An exclusive lock can only by obtained by a briefcase if no other briefcases hold shared
//! locks on the same object.
//!
//! An exclusive lock reserves the right for the owning briefcase to make any modifications
//! to the locked object or its contents; and prohibits all other briefcases from doing the same.
//! A shared lock simply prohibits any other briefcase from obtaining an exclusive lock,
//! reserving the right for the owning briefcases to acquire locks on any of its contents.
//! Obtaining an exclusive lock on a component of another lockable object implicitly obtains
//! a shared lock on the containing object.
//!
//! In general, locking should be performed at the most granular level possible, to ensure
//! efficient collaboration between briefcases.
//! Therefore, exclusive locks are generally reserved for elements, with shared locks on 
//! models and the DgnDb acquired automatically as a side effect.
//!
//! An exclusive lock on a model prevents any other briefcase from inserting, updating, or
//! deleting elements within that model; and from modifying the model itself.
//!
//! An exclusive lock on a DgnDb prevents any other briefcase from inserting, updating, or
//! deleting anything within the DgnDb.
//! 
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
enum class LockLevel
{
    None, //!< No ownership
    Shared, //!< Shared ownership
    Exclusive, //!< Exclusive ownership
};

//=======================================================================================
//! Identifies a lockable object and a lock level.
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct DgnLock
{
private:
    LockableId m_id;
    LockLevel m_level;
public:
    DgnLock() : m_level(LockLevel::None) { } //!< Constructs a lock with an invalid ID.
    DgnLock(LockableId id, LockLevel level) : m_id(id), m_level(level) { } //!< Constructs a lock for the specified lockable object
    DgnLock(DgnElementId elemId, LockLevel level) : m_id(elemId), m_level(level) { } //!< Constructs a lock for an element
    DgnLock(DgnModelId modelId, LockLevel level) : m_id(modelId), m_level(level) { } //!< Constructs a lock for a model
    DgnLock(DgnDbCR db, LockLevel level) : m_id(db), m_level(level) { } //!< Constructs a lock for a DgnDb

    BeSQLite::BeInt64Id GetId() const { return m_id.GetId(); } //!< The ID of the lockable object
    LockLevel GetLevel() const { return m_level; } //!< The level of the lock
    LockableType GetType() const { return m_id.GetType(); } //!< The type of the lockable object
    LockableId GetLockableId() const { return m_id; } //!< The ID and type of the lockable object
    bool IsExclusive() const { return LockLevel::Exclusive == GetLevel(); } //!< Determine if this is an exclusive lock

    void SetLevel(LockLevel level) { m_level = level; } //!< Set the lock level
    void EnsureLevel(LockLevel minLevel) { if (minLevel > m_level) SetLevel(minLevel); } //!< Ensure the lock level is no lower than the specified level
    void Invalidate() { m_id.Invalidate(); m_level = LockLevel::None; } //!< Invalidates this DgnLock

    //! Compare two locks by ID for sorting purposes
    struct IdentityComparator
    {
        bool operator()(DgnLockCR lhs, DgnLockCR rhs) const { return lhs.GetLockableId() < rhs.GetLockableId(); }
    };

    DGNPLATFORM_EXPORT void ToJson(JsonValueR value) const; //!< Convert to JSON representation
    DGNPLATFORM_EXPORT bool FromJson(JsonValueCR value); //!< Attempt to initialize from JSON representation
};

//! A set of locks compared by identity, ignoring lock level
typedef bset<DgnLock, DgnLock::IdentityComparator> DgnLockSet;

//=======================================================================================
//! Describes the ownership of a lockable object
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct DgnLockOwnership
{
    struct BriefcaseIdComparator
    {
        bool operator()(BeSQLite::BeBriefcaseId const& lhs, BeSQLite::BeBriefcaseId const& rhs) const
            {
            return lhs.GetValue() < rhs.GetValue();
            }
    };

    typedef bset<BeSQLite::BeBriefcaseId, BriefcaseIdComparator> BeBriefcaseIdSet;
private:
    BeSQLite::BeBriefcaseId m_exclusiveOwner;
    BeBriefcaseIdSet m_sharedOwners;
public:
    //! Constructs with the specified (or no) exclusive owner and no shared owners
    explicit DgnLockOwnership(BeSQLite::BeBriefcaseId exclusiveOwner=BeSQLite::BeBriefcaseId()) : m_exclusiveOwner(exclusiveOwner) { }

    //! Sets exclusive ownership to the specified owner
    void SetExclusiveOwner(BeSQLite::BeBriefcaseId owner) { BeAssert(owner.IsValid()); m_sharedOwners.clear(); m_exclusiveOwner = owner; }

    //! Adds a shared owner
    void AddSharedOwner(BeSQLite::BeBriefcaseId owner) { BeAssert(owner.IsValid()); m_exclusiveOwner.Invalidate(); m_sharedOwners.insert(owner); }

    //! Returns the level at which the object is locked
    LockLevel GetLockLevel() const
        {
        return m_exclusiveOwner.IsValid() ? LockLevel::Exclusive : m_sharedOwners.empty() ? LockLevel::None : LockLevel::Shared;
        }

    BeBriefcaseIdSet const& GetSharedOwners() const { return m_sharedOwners; } //<! Returns the set of shared owners
    BeSQLite::BeBriefcaseId GetExclusiveOwner() const { return m_exclusiveOwner; } //<! Returns the exclusive owner

    void Reset() { m_exclusiveOwner.Invalidate(); m_sharedOwners.clear(); } //!< Resets to no ownership

    DGNPLATFORM_EXPORT void ToJson(JsonValueR value) const; //!< Convert to JSON representation
    DGNPLATFORM_EXPORT bool FromJson(JsonValueCR value); //!< Attempt to initialize from JSON representation
};

//=======================================================================================
//! Specifies a request to acquire one or more locks.
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct LockRequest
{
    //! Customizes information to be included in the response. Note that specifying certain options may require more work on the part of the server and/or
    //! more involved processing of the response by the client.
    enum class ResponseOptions
    {
        None = 0, //!< No special options
        DeniedLocks = 1 << 0, //!< If a request to acquire locks is denied, the response will include the current lock state of each denied lock
    };

    typedef DgnLockSet::const_iterator const_iterator;
    typedef const_iterator iterator;
private:
    typedef DgnLockSet::iterator set_iterator;

    DgnLockSet m_locks;
    ResponseOptions m_options;

    void InsertLock(LockableId id, LockLevel level);
public:
    //! Constructor
    explicit LockRequest(ResponseOptions options=ResponseOptions::None) : m_options(options) { }

    //! Insert a request to lock an eleemnt.
    //! @param[in]      element The element to insert
    //! @param[in]      level   The desired lock level.
    //! @remarks If the level is LockLevel::None, nothing is inserted.
    //! If the level is LockLevel::Exclusive, a shared lock is also inserted for the element's model and DgnDb
    //! If the element has already been inserted, its level will be upgraded to the specified level
    DGNPLATFORM_EXPORT void Insert(DgnElementCR element, LockLevel level);

    //! Insert a request to lock a model.
    //! @param[in]      model The model to insert
    //! @param[in]      level The desired lock level.
    //! @remarks If the level is LockLevel::None, nothing is inserted.
    //! If the level is LockLevel::Exclusive, a shared lock is also inserted for the model's DgnDb; and exclusive locks are inserted for all of the model's elements
    //! If the model has already been inserted, its level will be upgraded to the specified level
    DGNPLATFORM_EXPORT void Insert(DgnModelCR model, LockLevel level);

    //! Inserts a request to lock the DgnDb.
    //! @param[in]      db    The DgnDb to insert
    //! @param[in]      level The desired lock level.
    //! @remarks If the level is LockLevel::None, nothing is inserted
    DGNPLATFORM_EXPORT void Insert(DgnDbCR db, LockLevel level);

    //! Inserts a collection of elements or models into the request at the specified level.
    //! @param[in]      lockableObjects A collection of elements or models
    //! @param[in]      level           The level of ownership requested
    template<typename T> void InsertAllOf(T const& lockableObjects, LockLevel level)
        {
        for (auto const& lockableObject : lockableObjects)
            Insert(lockableObject, level);
        }

    ResponseOptions GetOptions() const { return m_options; } //!< Returns the options specifying how the response is to be formulated
    bool IsEmpty() const { return m_locks.empty(); } //!< Determine if this request contains no locks
    size_t Size() const { return m_locks.size(); } //!< Returns the number of locks in this request
    void Clear() { m_locks.clear(); } //!< Removes all locks from this request
    DgnLockSet const& GetLockSet() const { return m_locks; }
    DgnLockSet& GetLockSet() { return m_locks; }

    void SetOptions(ResponseOptions options) { m_options = options; } //!< Sets the options specifying how the response is to be formulated

    //! Looks up a lock.
    //! @param[in]      lock            Specifies the ID and lock level to find
    //! @param[in]      matchExactLevel If true, compares the requested and found levels for exact equality
    //! @return The lock, or nullptr if no such lock exists
    //! @remarks If matchExactLevel is true, and the found lock's level is not exactly equal to the level of the input lock, returns nullptr
    //! If matchExactLevel is false, and the found lock's level is lower than the level of the input lock, returns nullptr
    DGNPLATFORM_EXPORT DgnLockCP Find(DgnLockCR lock, bool matchExactLevel=false) const;
    DgnLockCP Find(LockableId id) const { return Find(DgnLock(id, LockLevel::Exclusive)); } //!< Find a lock by ID

    bool Contains(DgnLockCR lock) const { return nullptr != Find(lock); } //!< Query whether the specified lock exists
    bool Contains(LockableId id) const { return nullptr != Find(id); } //!< Query whether an entry exists for the specified ID

    //! If an entry exists for the specified ID, return the associated LockLevel, or else LockLevel::None
    LockLevel GetLockLevel(LockableId id) const { auto found = Find(id); return nullptr != found ? found->GetLevel() : LockLevel::None; }

    const_iterator begin() const { return m_locks.begin(); } //!< An iterator to the first entry
    const_iterator end() const { return m_locks.end(); } //!< An iterator just beyond the last entry

    //! Removes the lock with the specified ID, if it exists.
    //! @param[in]      id 
    DGNPLATFORM_EXPORT void Remove(LockableId id);

    DGNPLATFORM_EXPORT void ToJson(JsonValueR value) const; //!< Convert to JSON representation
    DGNPLATFORM_EXPORT bool FromJson(JsonValueCR value); //!< Attempt to initialize from JSON representation

    //! A response from the server for a request to acquire locks
    struct Response
    {
    private:
        LockStatus  m_status;
        DgnLockSet  m_denied;
    public:
        explicit Response(LockStatus status=LockStatus::InvalidResponse) : m_status(status) { }

        LockStatus GetStatus() const { return m_status; } //!< The status code returned by the server

        //! If the request was denied, and the client specified ResponseOptions::DeniedLocks, returns the set of locks which were not granted because they were already held by another briefcase.
        DgnLockSet const& GetDeniedLocks() const { return m_denied; }
        DgnLockSet& GetDeniedLocks() { return m_denied; } //!< Returns a writable reference to the set of denied locks

        void Invalidate() { m_status = LockStatus::InvalidResponse; m_denied.clear(); } //!< Invalidate this response

        DGNPLATFORM_EXPORT void ToJson(JsonValueR value) const; //!< Convert to JSON representation
        DGNPLATFORM_EXPORT bool FromJson(JsonValueCR value); //!< Attempt to initialize from JSON representation
    };

    void FromChangeSummary(DgnChangeSummary const& changes, bool stopOnFirst=false); //!< @private
    void ExtractLockSet(DgnLockSet& locks); //!< @private
    DGNPLATFORM_EXPORT void FromRevision(DgnRevision& revision); //!< @private
};

ENUM_IS_FLAGS(LockRequest::ResponseOptions);

//=======================================================================================
//! Manages the acquisition of element and model locks for a briefcase.
//! In order to modify an object like an element or a model, a DgnDb (aka "briefcase"
//! must hold the lock for it. Once acquired, a lock remains owned by a briefcase until
//! explicitly relinquished, typically when committing changes to the central repository; or
//! when abandonding the briefcase's local changes.
//! The locks for all objects created by a briefcase are implicitly owned by that briefcase
//! until committed to the repository.
//! In some cases, modifying one element may cause indirect changes to other elements
//! through dependency handlers, in which case the briefcase must hold the locks for all
//! dependent elements.
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ILocksManager : RefCountedBase
{
private:
    DgnDbR  m_db;

    void RemoveElements(LockRequestR request, DgnModelId modelId) const;
protected:
    ILocksManager(DgnDbR db) : m_db(db) { }

    virtual bool _QueryLocksHeld(LockRequestR locks, bool localQueryOnly, LockStatus* status) = 0;
    virtual LockRequest::Response _AcquireLocks(LockRequestR locks) = 0;
    virtual LockStatus _RelinquishLocks() = 0;
    virtual LockStatus _DemoteLocks(DgnLockSet& locks) = 0;
    virtual LockStatus _QueryLockLevel(LockLevel& level, LockableId lockId, bool localQueryOnly) = 0;
    virtual LockStatus _RefreshLocks() = 0;

    virtual void _OnElementInserted(DgnElementId id) = 0;
    virtual void _OnModelInserted(DgnModelId id) = 0;

    DGNPLATFORM_EXPORT virtual LockStatus _LockElement(DgnElementCR el, LockLevel level, DgnModelId originalModelId);
    DGNPLATFORM_EXPORT virtual LockStatus _LockModel(DgnModelCR model, LockLevel level);

    DGNPLATFORM_EXPORT ILocksServerP GetLocksServer() const;
public:
    DgnDbR GetDgnDb() const { return m_db; }

    //! Returns true if this briefcase owns all of the requested locks. Note this function may modify the LockRequest object.
    //! This method always returns false if an error occurs while processing the query; check the optional LockStatus argument.
    bool QueryLocksHeld(LockRequestR locks, bool localQueryOnly=false, LockStatus* status=nullptr) { return _QueryLocksHeld(locks, localQueryOnly, status); }

    //! Attempts to acquire the specified locks. Note this function may modify the LockRequest object.
    LockRequest::Response AcquireLocks(LockRequestR locks) { return _AcquireLocks(locks); }

    //! Relinquishes all locks held by the DgnDb.
    LockStatus RelinquishLocks() { return _RelinquishLocks(); }

    //! Attempts to release the specified locks, or reduce the level at which the lock is held.
    //! Note this function may modify the contents of the DgnLockSet object.
    //! This method will fail if:
    //!  - Any pending/dynamics transactions exist in the managed DgnDb. They must first be committed or abandoned
    //!  - Any lock being released is required for changes made in the managed DgnDb. e.g., you cannot release a lock on an element you have modified.
    //! If this method succeeds, the undo/redo history will be reset for the managed DgnDb.
    LockStatus DemoteLocks(DgnLockSet& locks) { return _DemoteLocks(locks); }

    //! Refreshes any local cache of owned locks by re-querying the server
    LockStatus RefreshLocks() { return _RefreshLocks(); }

    //! Query this DgnDb's level of ownership of the specified lockable object.
    LockStatus QueryLockLevel(LockLevel& level, LockableId lockId, bool localQueryOnly=false) { return _QueryLockLevel(level, lockId, localQueryOnly); }
    //! Directly query the DgnDb's level of ownership of the specified lockable object.
    LockLevel QueryLockLevel(LockableId lockId, bool localOnly=false) { LockLevel level; return LockStatus::Success == QueryLockLevel(level, lockId, localOnly) ? level : LockLevel::None; }
    //! Query ownership of the specified DgnDb
    LockLevel QueryLockLevel(DgnDbCR db, bool localOnly=false) { return QueryLockLevel(LockableId(db), localOnly); }
    //! Query ownership of the specified element
    LockLevel QueryLockLevel(DgnElementCR el, bool localOnly=false) { return QueryLockLevel(LockableId(el), localOnly); }
    //! Query ownership of the specified model
    LockLevel QueryLockLevel(DgnModelCR model, bool localOnly=false) { return QueryLockLevel(LockableId(model), localOnly); }

    void OnElementInserted(DgnElementId id); //<! Invoked when a new element is inserted into the DgnDb
    void OnModelInserted(DgnModelId id); //<! Invoked when a new model is inserted into the DgnDb

    LockStatus LockElement(DgnElementCR el, LockLevel level, DgnModelId originalModelId=DgnModelId()); //!< Used internally to lock an element for direct changes.
    LockStatus LockModel(DgnModelCR model, LockLevel level); //!< Used internally to lock a model for direct changes.
    LockStatus LockDb(LockLevel level); //!< Used internally to lock the DgnDb

    //! Reformulate a denied request such that it does not contain any of the locks in the "denied" set.
    //! If the request contains locks which are dependent upon other locks in the denied set (e.g., elements within a model for which the model lock was not granted),
    //! the dependent locks will be removed.
    DGNPLATFORM_EXPORT void ReformulateRequest(LockRequestR request, DgnLockSet const& deniedLocks) const;

//__PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT static void BackDoor_SetLockingEnabled(bool enable);
//__PUBLISH_SECTION_START__
};

//=======================================================================================
//! Interface adopted by a server-like object which can coordinate locks held by multiple
//! briefcases.
//! In general, application code should interact with the ILocksManager object for a given
//! briefcase via DgnDb::Locks(). The ILocksManager will communicate with ILocksServer
//! as required.
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ILocksServer
{
protected:
    virtual LockStatus _QueryLocksHeld(bool& held, LockRequestCR locks, DgnDbR db) = 0;
    virtual LockRequest::Response _AcquireLocks(LockRequestCR locks, DgnDbR db) = 0;
    virtual LockStatus _RelinquishLocks(DgnDbR db) = 0;
    virtual LockStatus _DemoteLocks(DgnLockSet const& locks, DgnDbR db) = 0;
    virtual LockStatus _QueryLockLevel(LockLevel& level, LockableId lockId, DgnDbR db) = 0;
    virtual LockStatus _QueryLocks(DgnLockSet& locks, DgnDbR db) = 0;
    virtual LockStatus _QueryOwnership(DgnLockOwnershipR ownership, LockableId lockId) = 0;
public:
    //! Query whether all specified locks are held at or above the specified levels by the specified briefcase
    LockStatus QueryLocksHeld(bool& held, LockRequestCR locks, DgnDbR db) { return _QueryLocksHeld(held, locks, db); }

    //! Attempts to acquire the specified locks for the specified briefcase
    LockRequest::Response AcquireLocks(LockRequestCR locks, DgnDbR db) { return _AcquireLocks(locks, db); }

    //! Relinquishes all locks owned by a briefcase
    LockStatus RelinquishLocks(DgnDbR db) { return _RelinquishLocks(db); }

    //! Reduces the level at which a briefcase owns a set of locks.
    LockStatus DemoteLocks(DgnLockSet const& locks, DgnDbR db) { return _DemoteLocks(locks, db); }

    //! Queries the briefcase's level of ownership over the specified lockable object.
    LockStatus QueryLockLevel(LockLevel& level, LockableId lockId, DgnDbR db) { return _QueryLockLevel(level, lockId, db); }

    //! Attempts to retrieve the set of all locks held by a given briefcase
    LockStatus QueryLocks(DgnLockSet& locks, DgnDbR db) { return _QueryLocks(locks, db); }

    //! Queries the ownership of a lockable object
    LockStatus QueryOwnership(DgnLockOwnershipR ownership, LockableId lockId) { return _QueryOwnership(ownership, lockId); }
};

//=======================================================================================
//! Utilities for converting lock-related values to/from JSON.
//! See also To/FromJson() methods on classes like LockRequest, LockableId, etc.
// @bsiclass                                                      Paul.Connelly   12/15
//=======================================================================================
struct DgnLocksJson
{
public:
    DGNPLATFORM_EXPORT static bool BriefcaseIdFromJson(BeSQLite::BeBriefcaseId& id, JsonValueCR value);
    DGNPLATFORM_EXPORT static bool BeInt64IdFromJson(BeSQLite::BeInt64Id& id, JsonValueCR value);
    DGNPLATFORM_EXPORT static bool LockLevelFromJson(LockLevel& level, JsonValueCR value);
    DGNPLATFORM_EXPORT static bool LockableTypeFromJson(LockableType& type, JsonValueCR value);
    DGNPLATFORM_EXPORT static bool LockStatusFromJson(LockStatus& status, JsonValueCR value);

    DGNPLATFORM_EXPORT static void BriefcaseIdToJson(JsonValueR value, BeSQLite::BeBriefcaseId id);
    DGNPLATFORM_EXPORT static void BeInt64IdToJson(JsonValueR value, BeSQLite::BeInt64Id id);
    DGNPLATFORM_EXPORT static void LockLevelToJson(JsonValueR value, LockLevel level);
    DGNPLATFORM_EXPORT static void LockableTypeToJson(JsonValueR value, LockableType type);
    DGNPLATFORM_EXPORT static void LockStatusToJson(JsonValueR value, LockStatus status);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE


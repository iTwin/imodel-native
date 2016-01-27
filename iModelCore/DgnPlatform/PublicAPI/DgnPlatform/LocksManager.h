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
DGNPLATFORM_TYPEDEFS(DgnOwnedLock);

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
//! Describes a lock and its ownership.
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct DgnOwnedLock
{
private:
    DgnLockOwnership    m_ownership;
    LockableId          m_id;
public:
    DgnOwnedLock() { } //!< Default constructor, chiefly for use by stl containers
    explicit DgnOwnedLock(LockableId lockId) : m_id(lockId) { } //!< Construct for the specified lock, with ownership yet to be supplied

    LockableId GetLockableId() const { return m_id; } //!< The ID of the lockable object
    DgnLockOwnershipCR GetOwnership() const { return m_ownership; } //!< The ownership of the lock
    DgnLockOwnershipR GetOwnership() { return m_ownership; } //!< A writable reference to the ownership of the lock

    //! Compare two DgnOwnedLock objects by the ID of the lockable object, for sorting purposes
    struct IdentityComparator
    {
        bool operator()(DgnOwnedLockCR lhs, DgnOwnedLockCR rhs) const { return lhs.GetLockableId() < rhs.GetLockableId(); }
    };
};

//! A set of locks paired with their ownership details, compared by identity of the locked object
typedef bset<DgnOwnedLock, DgnOwnedLock::IdentityComparator> DgnOwnedLockSet;

//=======================================================================================
//! Specifies a request to acquire one or more locks.
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct LockRequest
{
    typedef DgnLockSet::const_iterator const_iterator;
    typedef const_iterator iterator;
private:
    typedef DgnLockSet::iterator set_iterator;

    DgnLockSet m_locks;

    void InsertLock(LockableId id, LockLevel level);
public:
    //! Constructor
    LockRequest() { }

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

    bool IsEmpty() const { return m_locks.empty(); } //!< Determine if this request contains no locks
    size_t Size() const { return m_locks.size(); } //!< Returns the number of locks in this request
    void Clear() { m_locks.clear(); } //!< Removes all locks from this request
    DgnLockSet const& GetLockSet() const { return m_locks; }
    DgnLockSet& GetLockSet() { return m_locks; }

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

    void FromChangeSummary(DgnChangeSummary const& changes, bool stopOnFirst=false); //!< @private
    void ExtractLockSet(DgnLockSet& locks); //!< @private
    DGNPLATFORM_EXPORT void FromRevision(DgnRevision& revision); //!< @private
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
    DGNPLATFORM_EXPORT static bool RepositoryStatusFromJson(RepositoryStatus& status, JsonValueCR value);

    DGNPLATFORM_EXPORT static void BriefcaseIdToJson(JsonValueR value, BeSQLite::BeBriefcaseId id);
    DGNPLATFORM_EXPORT static void BeInt64IdToJson(JsonValueR value, BeSQLite::BeInt64Id id);
    DGNPLATFORM_EXPORT static void LockLevelToJson(JsonValueR value, LockLevel level);
    DGNPLATFORM_EXPORT static void LockableTypeToJson(JsonValueR value, LockableType type);
    DGNPLATFORM_EXPORT static void RepositoryStatusToJson(JsonValueR value, RepositoryStatus status);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE


/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/LocksManager.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatform.h>

DGNPLATFORM_TYPEDEFS(LockRequest);
DGNPLATFORM_TYPEDEFS(DgnLock);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

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

    //! Compare two LockableIds for sorting purposes
    bool operator<(LockableId const& rhs) const
        {
        return m_type != rhs.m_type ? m_type < rhs.m_type : m_id < rhs.m_id;
        }
};

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

    //! Compare two locks by ID for sorting purposes
    struct IdentityComparator
    {
        bool operator()(DgnLockCR lhs, DgnLockCR rhs) const { return lhs.GetLockableId() < rhs.GetLockableId(); }
    };
};

//! A set of locks compared by identity, ignoring lock level
typedef bset<DgnLock, DgnLock::IdentityComparator> DgnLockSet;

//=======================================================================================
//! Specifies a request for one or more locks.
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

    //! Populate a LockRequest object from the set of changes in a DgnDb, containing all locks required for the actual changes made.
    DGNPLATFORM_EXPORT DgnDbStatus FromChangeSet(BeSQLite::IChangeSet& changes, DgnDbR db);

    //! Remove any locks from this request which are also present (at any lock level) in the supplied request, returning the number of locks removed.
    DGNPLATFORM_EXPORT size_t Subtract(LockRequestCR request);
};

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
protected:
    ILocksManager(DgnDbR db) : m_db(db) { }

    virtual bool _QueryLocksHeld(LockRequestR locks, bool localQueryOnly) = 0;
    virtual LockStatus _AcquireLocks(LockRequestR locks) = 0;
    virtual LockStatus _RelinquishLocks() = 0;
    virtual LockStatus _QueryLockLevel(LockLevel& level, LockableId lockId, bool localQueryOnly) = 0;
    virtual LockStatus _RefreshLocks() = 0;

    virtual void _OnElementInserted(DgnElementId id) = 0;
    virtual void _OnModelInserted(DgnModelId id) = 0;

    DGNPLATFORM_EXPORT virtual LockStatus _LockElement(DgnElementCR el, LockLevel level, DgnModelId originalModelId);
    DGNPLATFORM_EXPORT virtual LockStatus _LockModel(DgnModelCR model, LockLevel level);

    DGNPLATFORM_EXPORT BeFileName GetLockTableFileName() const;
    DGNPLATFORM_EXPORT ILocksServerP GetLocksServer() const;
public:
    DgnDbR GetDgnDb() const { return m_db; }

    //! Returns true if this briefcase owns all of the requested locks. Note this function may modify the LockRequest object.
    bool QueryLocksHeld(LockRequestR locks, bool localQueryOnly=false) { return _QueryLocksHeld(locks, localQueryOnly); }

    //! Attempts to acquire the specified locks. Note this function may modify the LockRequest object.
    LockStatus AcquireLocks(LockRequestR locks) { return _AcquireLocks(locks); }

    //! Relinquishes all locks held by the DgnDb.
    LockStatus RelinquishLocks() { return _RelinquishLocks(); }

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
    virtual bool _QueryLocksHeld(LockRequestCR locks, DgnDbR db) = 0;
    virtual LockStatus _AcquireLocks(LockRequestCR locks, DgnDbR db) = 0;
    virtual LockStatus _RelinquishLocks(DgnDbR db) = 0;
    virtual LockStatus _QueryLockLevel(LockLevel& level, LockableId lockId, DgnDbR db) = 0;
    virtual LockStatus _QueryLocks(DgnLockSet& locks, DgnDbR db) = 0;
public:
    //! Returns true if all specified locks are held at or above the specified levels by the specified briefcase
    bool QueryLocksHeld(LockRequestCR locks, DgnDbR db) { return _QueryLocksHeld(locks, db); }

    //! Attempts to acquire the specified locks for the specified briefcase
    LockStatus AcquireLocks(LockRequestCR locks, DgnDbR db) { return _AcquireLocks(locks, db); }

    //! Relinquishes all locks owned by a briefcase
    LockStatus RelinquishLocks(DgnDbR db) { return _RelinquishLocks(db); }

    //! Queries the briefcase's level of ownership over the specified lockable object.
    LockStatus QueryLockLevel(LockLevel& level, LockableId lockId, DgnDbR db) { return _QueryLockLevel(level, lockId, db); }

    //! Attempts to retrieve the set of all locks held by a given briefcase
    LockStatus QueryLocks(DgnLockSet& locks, DgnDbR db) { return _QueryLocks(locks, db); }
};

END_BENTLEY_DGNPLATFORM_NAMESPACE


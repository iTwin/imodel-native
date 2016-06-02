/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/RepositoryManager.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/LocksManager.h>
#include <DgnPlatform/DgnCodesManager.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=====================================================================================*/
//! Manages repository-allocated resources held by a DgnDb - namely, Locks and Codes.
//!
//! In order to directly modify an object like an element or model, a DgnDb (aka "briefcase"
//! must hold the lock for it. If the lock is not held, the briefcase must acquire it
//! from the central repository via the IRepositoryManager interface.
//! Once acquired, a lock remains owned by a briefcase until
//! explicitly relinquished - e.g., when committing changes to the central repository, or
//! abandoning the briefcase's local changes.
//! The locks for all objects created by a briefcase are implicitly owned by that briefcase
//! until committed to the repository.
//!
//! @see DgnCodeState
//! @see LockLevel
//! @see LockRequest
//! @see IRepositoryManager
// @bsistruct                                                    Paul.Connelly   01/16
/*=====================================================================================*/
struct IBriefcaseManager : RefCountedBase
{
public:
    //! Enumerates the types of resources managed by a briefcase
    enum class Resources : uint8_t
    {
        None = 0, //!< None
        Locks = 1 << 0, //!< Locks
        Codes = 1 << 1, //!< Codes
        All = 0xff //!< Both Locks and Codes
    };

    //! Allow a request to specify options for what additional information is to be included in the response
    enum class ResponseOptions : uint8_t
    {
        None = 0, //!< No special options
        LockState = 1 << 0, //!< If a request to acquire locks is denied, the response will include the current lock state of each denied lock
        CodeState = 1 << 1, //!< Include DgnCodeState for any codes for which the request was denied
        RevisionIds = 1 << 2, //!< For locks or codes requiring a revision to be pulled, include the specific revision IDs.
        All = 0xff, //!< Include all options
    };

    //! Possible actions performed by various PrepareFor* methods
    enum class PrepareAction
    {
        //! Populate the set of locks and codes required for a given operation
        //! @note This action does not contact the repository manager
        Populate,
        //! Populate the set of locks and codes required for a given operation, and then verify that all are already held by this briefcase
        //! @note This action does not contact the repository manager
        Verify,
        //! Populate the set of locks and codes required for a given operation, and then request them from the repository manager
        //! @note This action may contact the repository manager. If multiple operations are to be performed - e.g., modification of several elements - it is far more efficient to populate a single Request from all operations before forwarding to the repository manager.
        Acquire,
    };

    //! A request made to the IBriefcaseManager and possibly forwarded to the IRepositoryManager.
    //! Specifies a set of locks the briefcase wishes to acquire and/or a set of codes to be reserved.
    struct Request
    {
    private:
        LockRequest     m_locks;
        DgnCodeSet      m_codes;
        ResponseOptions m_options;
    public:
        //! Construct an empty request with the specified options
        explicit Request(ResponseOptions opts=ResponseOptions::None) : m_options(opts) { }

        LockRequest const& Locks() const { return m_locks; } //!< The locks to be acquired
        DgnCodeSet const& Codes() const { return m_codes; } //!< The codes to be reserved
        ResponseOptions Options() const { return m_options; } //!< The options for customizing the response to this request
        bool IsEmpty() const { return m_locks.IsEmpty() && m_codes.empty(); } //!< Returns true if this request contains no codes and no locks

        LockRequest& Locks() { return m_locks; } //!< A writable reference to the requested locks
        DgnCodeSet& Codes() { return m_codes; } //!< A writable reference to the requested codes
        void SetOptions(ResponseOptions opts) { m_options = opts; } //!< Customize the data to be included in the response to this request

        void Reset() { m_options = ResponseOptions::None; m_codes.clear(); m_locks.Clear(); }

        DGNPLATFORM_EXPORT void ToJson(JsonValueR value) const; //!< Convert to JSON representation
        DGNPLATFORM_EXPORT bool FromJson(JsonValueCR value); //!< Attempt to initialize from JSON representation
    };

    //! A response to a Request, containing the overall result as a RepositoryStatus, and any additional information as specified by ResponseOptions accompanying the request
    struct Response
    {
    private:
        DgnCodeInfoSet      m_codeStates;
        DgnLockInfoSet      m_lockStates;
        RepositoryStatus    m_status;
    public:
        //! Construct with the specified result
        explicit Response(RepositoryStatus status=RepositoryStatus::InvalidResponse) : m_status(status) { }

        RepositoryStatus Result() const { return m_status; } //!< The result of the operation
        DgnLockInfoSet const& LockStates() const { return m_lockStates; } //!< The states of any locks which could not be acquired, if ResponseOptions::LockState was specified
        DgnCodeInfoSet const& CodeStates() const { return m_codeStates; } //!< The states of any codes which could not be reserved, if ResponseOptions::CodeState was specified

        void SetResult(RepositoryStatus result) { m_status = result; } //!< Set the result of the operation
        DgnLockInfoSet& LockStates() { return m_lockStates; } //!< A writable reference to the lock states
        DgnCodeInfoSet& CodeStates() { return m_codeStates; } //!< A writable reference to the code states

        //! Reset to default state ("invalid response")
        void Invalidate() { m_status = RepositoryStatus::InvalidResponse; m_lockStates.clear(); m_codeStates.clear(); }

        DGNPLATFORM_EXPORT void ToJson(JsonValueR value) const; //!< Convert to JSON representation
        DGNPLATFORM_EXPORT bool FromJson(JsonValueCR value); //!< Attempt to initialize from JSON representation
    };
        
private:
    DgnDbR  m_db;

    void ReformulateLockRequest(LockRequestR, Response const&) const;
    void ReformulateCodeRequest(DgnCodeSet&, Response const&) const;
    void RemoveElements(LockRequestR, DgnModelId) const;
protected:
    explicit IBriefcaseManager(DgnDbR db) : m_db(db) { }

    // Codes and Locks
    virtual Response _ProcessRequest(Request& request) = 0;
    virtual RepositoryStatus _Demote(DgnLockSet& locks, DgnCodeSet const& codes) = 0;
    virtual RepositoryStatus _Relinquish(Resources which) = 0;
    virtual bool _AreResourcesHeld(DgnLockSet& locks, DgnCodeSet& codes, RepositoryStatus* status) = 0;
    virtual RepositoryStatus _PrepareForElementOperation(Request& req, DgnElementCR el, BeSQLite::DbOpcode opcode, DgnElementCP original) = 0;
    virtual RepositoryStatus _PrepareForModelOperation(Request& req, DgnModelCR model, BeSQLite::DbOpcode opcode) = 0;

    // Codes
    virtual RepositoryStatus _QueryCodeStates(DgnCodeInfoSet& states, DgnCodeSet const& codes) = 0;
    DGNPLATFORM_EXPORT virtual RepositoryStatus _ReserveCode(DgnCodeCR code);

    // Locks
    virtual RepositoryStatus _QueryLockLevel(LockLevel& level, LockableId lockId) = 0;
    virtual RepositoryStatus _QueryLockLevels(DgnLockSet& lockLevels, LockableIdSet& lockIds) = 0;

    // Local state management
    virtual RepositoryStatus _OnFinishRevision(DgnRevision const& rev) = 0;
    virtual void _OnElementInserted(DgnElementId id) = 0;
    virtual void _OnModelInserted(DgnModelId id) = 0;
    virtual RepositoryStatus _RefreshFromRepository() = 0;
    virtual void _OnDgnDbDestroyed() { }

    DGNPLATFORM_EXPORT IRepositoryManagerP GetRepositoryManager() const;
    DGNPLATFORM_EXPORT bool LocksRequired() const;
    DGNPLATFORM_EXPORT RepositoryStatus PrepareForElementOperation(Request& req, DgnElementCR el, BeSQLite::DbOpcode opcode, PrepareAction action, DgnElementCP original=nullptr);
    DGNPLATFORM_EXPORT RepositoryStatus PrepareForModelOperation(Request& req, DgnModelCR model, BeSQLite::DbOpcode opcode, PrepareAction action);
    RepositoryStatus PerformAction(Request& req, PrepareAction action);
    DgnDbStatus OnElementOperation(DgnElementCR el, BeSQLite::DbOpcode opcode, DgnElementCP pre=nullptr);
    DgnDbStatus OnModelOperation(DgnModelCR model, BeSQLite::DbOpcode opcode);
    static DgnDbStatus ToDgnDbStatus(RepositoryStatus repoStatus, Request const& request);
public:
    DgnDbR GetDgnDb() const { return m_db; } //!< The DgnDb managed by this object

    //! @name Managing both Locks and Codes
    //@{
    //! Process the request and return a response, forwarding the request to the repository manager if required.
    //! @param[in]      request The set of locks and/or codes to acquire, and options for customizing the response
    //! @return The response, containing the result and any additional details as specified by the request's ResponseOptions
    //! @remarks Note the contents of the request may be modified.
    Response ProcessRequest(Request& request) { return _ProcessRequest(request); }

    //! Demote the previously-acquired locks to the specified levels, and release the previously-reserved codes
    //! @param[in]      locks The set of locks to demote and their new levels
    //! @param[in]      codes The set of codes to release
    //! @return Success, or an error code
    //! @remarks This method cannot be used to increase a lock's level. It cannot be used to release locks or codes required by the briefcase's local changes.
    RepositoryStatus Demote(DgnLockSet& locks, DgnCodeSet const& codes) { return _Demote(locks, codes); }

    //! Relinquish all reserved codes and/or all held locks
    //! @param[in]      which The type(s) of resources to relinquish
    //! @return Success, or an error status
    //! @remarks This method cannot be used to release locks or codes required by the briefcase's local changes.
    RepositoryStatus Relinquish(Resources which=Resources::All) { return _Relinquish(which); }

    //! Returns whether the specified resources are held by this briefcase
    //! @param[in]      locks  A set of locks and levels
    //! @param[in]      codes  A set of codes
    //! @param[in]      status If non-null, receives the result of the operation
    //! @return True if the operation completed successfully, all of the locks are held at or above the desired level, and all of the codes are reserved.
    //! @remarks The sets of locks and codes may be modified.
    bool AreResourcesHeld(DgnLockSet& locks, DgnCodeSet& codes, RepositoryStatus* status=nullptr) { return _AreResourcesHeld(locks, codes, status); }

    //! Populates the request with the locks + codes required to insert the specified element into the DgnDb
    RepositoryStatus PrepareForElementInsert(Request& request, DgnElementCR el, PrepareAction action=PrepareAction::Populate) { return PrepareForElementOperation(request, el, BeSQLite::DbOpcode::Insert, action); }

    //! Populates the request with the locks + codes required to update the specified element in the DgnDb
    DGNPLATFORM_EXPORT RepositoryStatus PrepareForElementUpdate(Request& request, DgnElementCR el, PrepareAction action=PrepareAction::Populate);

    //! Populates the request with the locks + codes required to delete the specified element from the DgnDb
    RepositoryStatus PrepareForElementDelete(Request& request, DgnElementCR el, PrepareAction action=PrepareAction::Populate) { return PrepareForElementOperation(request, el, BeSQLite::DbOpcode::Delete, action); }

    //! Populates the request with the locks + codes required to insert the specified model into the DgnDb
    RepositoryStatus PrepareForModelInsert(Request& request, DgnModelCR model, PrepareAction action=PrepareAction::Populate) { return PrepareForModelOperation(request, model, BeSQLite::DbOpcode::Insert, action); }

    //! Populates the request with the locks + codes required to update the specified model in the DgnDb
    RepositoryStatus PrepareForModelUpdate(Request& request, DgnModelCR model, PrepareAction action=PrepareAction::Populate) { return PrepareForModelOperation(request, model, BeSQLite::DbOpcode::Update, action); }

    //! Populates the request with the locks + codes required to delete the specified model from the DgnDb
    RepositoryStatus PrepareForModelDelete(Request& request, DgnModelCR model, PrepareAction action=PrepareAction::Populate) { return PrepareForModelOperation(request, model, BeSQLite::DbOpcode::Delete, action); }
    //@}

    //! @name Managing Locks
    //{
    //! Returns whether the specified locks are held by this briefcase
    //! @param[in]      locks  The set of locks and their desired levels
    //! @param[in]      status If non-null, receives the result of the operation
    //! @return True if the operation completed successfully and all of the locks are held at or above the desired level
    //! @remarks The set of locks may be modified.
    bool AreLocksHeld(DgnLockSet& locks, RepositoryStatus* status=nullptr) { DgnCodeSet codes; return AreResourcesHeld(locks, codes, status); }

    //! Releases all locks held by this briefcase
    //! @return Success, or an error status
    //! @remarks This method cannot be used to release locks required by the briefcase's local changes.
    RepositoryStatus RelinquishLocks() { return _Relinquish(Resources::Locks); }

    //! Reduces the levels at which a set of locks are held
    //! @param[in]      locks The set of locks and their new levels
    //! @return Success, or an error status
    //! @remarks This method cannot be used to increase a lock's level or to release locks required by the briefcase's local changes. The DgnLockSet may be modified
    RepositoryStatus DemoteLocks(DgnLockSet& locks) { return _Demote(locks, DgnCodeSet()); }

    //! Attempts to acquire a set of locks
    //! @param[in]      locks   The locks to acquire
    //! @param[in]      options Options for customizing the data included in the response
    //! @return The response to the request
    //! @remarks The LockRequest may be modified
    DGNPLATFORM_EXPORT Response AcquireLocks(LockRequestR locks, ResponseOptions options=ResponseOptions::None);

    //! Query the level at which a lock is held by this briefcase
    //! @param[in]      level   If successful, upon return holds the level at which this briefcase holds the lock
    //! @param[in]      lockId  The lock to query
    //! @return Success, or an error status
    RepositoryStatus QueryLockLevel(LockLevel& level, LockableId lockId) { return _QueryLockLevel(level, lockId); }

    //! Query the levels at which each of the specified locks are held by this briefcase
    //! @param[in]      lockLevels If successful, upon return holds the locks and the levels at which this briefcase holds them
    //! @param[in]      lockIds    The set of locks to query
    //! @return Success, or an error status
    //! @remarks The DgnLockSet and LockableIdSet may be modified
    RepositoryStatus QueryLockLevels(DgnLockSet& lockLevels, LockableIdSet& lockIds) { return _QueryLockLevels(lockLevels, lockIds); }

    //! Directly query the level at which a lock is held
    LockLevel QueryLockLevel(LockableId lockId) { LockLevel level; return RepositoryStatus::Success == QueryLockLevel(level, lockId) ? level : LockLevel::None; }
    LockLevel QueryLockLevel(DgnDbCR db) { return QueryLockLevel(LockableId(db)); } //!< Query the level at which the Db is locked by this briefcase
    LockLevel QueryLockLevel(DgnElementCR el) { return QueryLockLevel(LockableId(el)); } //!< Query the level at which the element is locked by this briefcase
    LockLevel QueryLockLevel(DgnModelCR model) { return QueryLockLevel(LockableId(model)); } //!< Query the level at which the model is locked by this briefcase

    //! Reformulate a denied request such that it does not contain any of the locks or codes in the "denied" sets.
    //! If the request contains locks which are dependent upon other locks in the denied set (e.g., elements within a model for which the model lock was not granted),
    //! the dependent locks will be removed.
    //! @param[in]      req      The original request to be reformulated
    //! @param[in]      response The response to the original request, containing the sets of locks and codes which could not be obtained
    //! @remarks This method is only effective if the original request specified ResponseOptions::LockState and ResponseOptions::CodeState
    DGNPLATFORM_EXPORT void ReformulateRequest(Request& req, Response const& response) const;

    //! Acquire a shared or exclusive lock on the DgnDb.
    Response LockDb(LockLevel level) { Request req; req.Locks().Insert(GetDgnDb(), level); return ProcessRequest(req); }
    //@}

    //! @name Managing Codes
    //@{
    //! Query the states of a set of codes
    //! @param[in]      states Upon successful return, holds the state of each code
    //! @param[in]      codes  The set of codes to query
    //! @return Success, or an error status
    RepositoryStatus QueryCodeStates(DgnCodeInfoSet& states, DgnCodeSet const& codes) { return _QueryCodeStates(states, codes); }

    //! Attempts to reserve a single code for use by this briefcase
    //! @param[in]      code The code to reserve
    //! @return Success if the code was reserved, or an error status
    RepositoryStatus ReserveCode(DgnCodeCR code) { return _ReserveCode(code); }

    //! Relinquishes all codes reserved by this briefcase
    //! @return Success, or an error status
    //! @remarks This method cannot be used to release codes required by the briefcase's local changes
    RepositoryStatus RelinquishCodes() { return _Relinquish(Resources::Codes); }

    //! Release a set of codes reserved by this briefcase
    //! @param[in]      codes The set of codes to release
    //! @return Success, or an error status
    //! @remarks This method cannot be used to release codes required by the briefcase's local changes
    RepositoryStatus ReleaseCodes(DgnCodeSet const& codes) { DgnLockSet locks; return _Demote(locks, codes); }

    //! Returns whether all of the codes have been reserved by this briefcase
    //! @param[in]      codes  The set of codes to query
    //! @param[in]      status If non-null, receives the result of the operation
    //! @return True if all codes have been previously reserved by this briefcase
    //! @remarks The DgnCodeSet may be modified.
    bool AreCodesReserved(DgnCodeSet& codes, RepositoryStatus* status=nullptr) { DgnLockSet locks; return AreResourcesHeld(locks, codes, status); }

    //! Attempts to reserve a set of codes for this briefcase's use
    //! @param[in]      codes   The desired codes
    //! @param[in]      options Options for customizing what data is included in the response
    //! @return The response, including the result status and any additional information as specified by the options
    //! @remarks The DgnCodeSet may be modified
    DGNPLATFORM_EXPORT Response ReserveCodes(DgnCodeSet& codes, ResponseOptions options=ResponseOptions::None);
    //@}

    //! @name Local State Management
    //@{
    //! Refreshes the state of this briefcase's resources from the server.
    //! @return Success, or an error status
    //! @remarks This is generally only useful if the repository was temporarily unavailable when previous requests were attempted.
    RepositoryStatus RefreshFromRepository() { return _RefreshFromRepository(); }
    RepositoryStatus OnFinishRevision(DgnRevision const& rev) { return _OnFinishRevision(rev); } //!< @private
    void OnElementInserted(DgnElementId id); //!< @private
    void OnModelInserted(DgnModelId id); //!< @private
    //@}

    DgnDbStatus OnElementInsert(DgnElementCR el); //!< @private
    DgnDbStatus OnElementUpdate(DgnElementCR el, DgnElementCR pre); //!< @private
    DgnDbStatus OnElementDelete(DgnElementCR el); //!< @private
    DgnDbStatus OnModelInsert(DgnModelCR model); //!< @private
    DgnDbStatus OnModelUpdate(DgnModelCR model); //!< @private
    DgnDbStatus OnModelDelete(DgnModelCR model); //!< @private
    void OnDgnDbDestroyed() { _OnDgnDbDestroyed(); } //!< @private
};

/*=====================================================================================*/
//! Communicates with the central repository to query and modify the state of resources
//! administered by the repository - namely, locks and codes.
//! In general, client code should prefer to communicate with an IBriefcaseManager and
//! allow it to forward requests to the IRepositoryManager as required.
//! Because the repository is administered from a server, its IRepositoryManager may not be
//! available or accessible in all contexts.
// @bsistruct                                                    Paul.Connelly   01/16
/*=====================================================================================*/
struct IRepositoryManager
{
    typedef IBriefcaseManager::Request Request;
    typedef IBriefcaseManager::Response Response;
    typedef IBriefcaseManager::Resources Resources;
    typedef IBriefcaseManager::ResponseOptions ResponseOptions;
protected:
    // Codes + Locks
    virtual Response _ProcessRequest(Request const& req, DgnDbR db) = 0;
    virtual RepositoryStatus _Demote(DgnLockSet const& locks, DgnCodeSet const& codes, DgnDbR db) = 0;
    virtual RepositoryStatus _Relinquish(Resources which, DgnDbR db) = 0;
    virtual RepositoryStatus _QueryHeldResources(DgnLockSet& locks, DgnCodeSet& codes, DgnDbR db) = 0;
    virtual RepositoryStatus _QueryStates(DgnLockInfoSet& lockStates, DgnCodeInfoSet& codeStates, LockableIdSet const& locks, DgnCodeSet const& codes) = 0;
public:
    //! Process a briefcase's request to acquire locks and/or reserve codes
    //! @param[in]      req The set of desired codes and/or locks, and options for customizing what data is included in the response
    //! @param[in]      db  The requesting briefcase
    //! @return The response, customized according to the specified options
    Response ProcessRequest(Request const& req, DgnDbR db) { return _ProcessRequest(req, db); }

    //! Reduces the level at which a set of locks are held and/or releases a set of reserved codes
    //! @param[in]      locks The set of locks to demote and their new levels
    //! @param[in]      codes The set of reserved codes to release
    //! @param[in]      db    The requesting briefcase
    //! @return Success, or an error status
    RepositoryStatus Demote(DgnLockSet const& locks, DgnCodeSet const& codes, DgnDbR db) { return _Demote(locks, codes, db); }

    //! Relinquish all locks and/or codes held by a briefcase
    //! @param[in]      which The type(s) of resources to relinquish
    //! @param[in]      db    The requesting briefcase
    //! @return Success, or an error status
    //! @remarks This method cannot be used to relinquish locks or codes required by the briefcase's local changes
    RepositoryStatus Relinquish(Resources which, DgnDbR db) { return _Relinquish(which, db); }

    //! Queries the repository state of a set of resources
    //! @param[in]      lockStates Upon successful return, holds the state of each specified lock
    //! @param[in]      codeStates Upon successful return, holds the state of each specified code
    //! @param[in]      locks      The set of locks to query
    //! @param[in]      codes      The set of codes to query
    //! @return Success, or an error status
    RepositoryStatus QueryStates(DgnLockInfoSet& lockStates, DgnCodeInfoSet& codeStates, LockableIdSet const& locks, DgnCodeSet const& codes) { return _QueryStates(lockStates, codeStates, locks, codes); }

    //! Retrieves the set of resources held by a briefcase as recorded in the repository
    //! @param[in]      locks The set of locks tracked by the repository and held by the briefcase
    //! @param[in]      codes The set of codes tracked by the repository and held by the briefcase
    //! @param[in]      db    The requesting briefcase
    //! @return Success, or an error status
    //! @remarks This method only returns resources tracked by the repository - e.g., excluding locks implicitly held for elements/models created locally by this briefcase and not yet committed to the repository
    RepositoryStatus QueryHeldResources(DgnLockSet& locks, DgnCodeSet& codes, DgnDbR db) { return _QueryHeldResources(locks, codes, db); }

    //! Query the repository states of a set of codes
    //! @param[in]      states Upon successful return, holds the state of each specified code
    //! @param[in]      codes  The set of codes to query
    //! @return Success, or an error status
    DGNPLATFORM_EXPORT RepositoryStatus QueryCodeStates(DgnCodeInfoSet& states, DgnCodeSet const& codes);

    //! Query the repository status of a set of locks
    //! @param[in]      states Upon successful return, holds the state of each specified lock
    //! @param[in]      locks  The set of locks to query
    //! @return Success, or an error status
    DGNPLATFORM_EXPORT RepositoryStatus QueryLockStates(DgnLockInfoSet& states, LockableIdSet const& locks);
};

ENUM_IS_FLAGS(IBriefcaseManager::ResponseOptions);
ENUM_IS_FLAGS(IBriefcaseManager::Resources);

//__PUBLISH_SECTION_END__
//=======================================================================================
//! Utilities for converting IRepositoryManager-related values to/from JSON.
//! See also To/FromJson() methods on classes like LockRequest, LockableId, etc.
// @bsiclass                                                      Paul.Connelly   12/15
//=======================================================================================
namespace RepositoryJson
{
    DGNPLATFORM_EXPORT bool BriefcaseIdFromJson(BeSQLite::BeBriefcaseId& id, JsonValueCR value);
    DGNPLATFORM_EXPORT bool BeInt64IdFromJson(BeInt64Id& id, JsonValueCR value);
    DGNPLATFORM_EXPORT bool LockLevelFromJson(LockLevel& level, JsonValueCR value);
    DGNPLATFORM_EXPORT bool LockableTypeFromJson(LockableType& type, JsonValueCR value);
    DGNPLATFORM_EXPORT bool RepositoryStatusFromJson(RepositoryStatus& status, JsonValueCR value);
    DGNPLATFORM_EXPORT bool ResponseOptionsFromJson(IBriefcaseManager::ResponseOptions& options, JsonValueCR value);

    DGNPLATFORM_EXPORT void BriefcaseIdToJson(JsonValueR value, BeSQLite::BeBriefcaseId id);
    DGNPLATFORM_EXPORT void BeInt64IdToJson(JsonValueR value, BeInt64Id id);
    DGNPLATFORM_EXPORT void LockLevelToJson(JsonValueR value, LockLevel level);
    DGNPLATFORM_EXPORT void LockableTypeToJson(JsonValueR value, LockableType type);
    DGNPLATFORM_EXPORT void RepositoryStatusToJson(JsonValueR value, RepositoryStatus status);
    DGNPLATFORM_EXPORT void ResponseOptionsToJson(JsonValueR value, IBriefcaseManager::ResponseOptions options);
} // namespace RepositoryJson
//__PUBLISH_SECTION_START__

END_BENTLEY_DGNPLATFORM_NAMESPACE


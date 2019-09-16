/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/LocksManager.h>
#include <DgnPlatform/DgnCodesManager.h>
#include <BeSQLite/ChangeSet.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! An iterator over the locks held by an IBriefcaseManager.
// @bsistruct                                                   Paul.Connelly   03/17
//=======================================================================================
struct IOwnedLocksIterator : RefCountedBase
{
protected:
    virtual DgnLockCR _GetCurrent() const = 0;
    virtual bool _IsAtEnd() const = 0;
    virtual void _MoveNext() = 0;
    virtual void _Reset() = 0;
public:
    DgnLockCR operator*() const { return _GetCurrent(); }
    DgnLockCP operator->() const { return &_GetCurrent(); }
    void operator++() { _MoveNext(); }
    bool IsValid() const { return !_IsAtEnd(); }
    void Reset() { _Reset(); }
};

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
struct EXPORT_VTABLE_ATTRIBUTE IBriefcaseManager : RefCountedBase
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
        UnlimitedReporting = 1 << 3, //! Acuire all denied instances despite server side thresholds
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

    //! Associated with a Response to identify the purpose for which a request was made
    enum class RequestPurpose
    {
        Acquire,    //!< Attempted to acquire locks/codes
        Query,      //!< Queried server for availability of locks/codes
        FastQuery,  //!< Queried local cache for availability of locks/codes. Response may not include full ownership details for denied request.
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
        RequestPurpose      m_purpose;
        ResponseOptions     m_options;
    public:
        //! Construct with the specified result
        Response(RequestPurpose purpose, ResponseOptions options, RepositoryStatus status=RepositoryStatus::InvalidResponse) : m_status(status) { }

        RepositoryStatus Result() const { return m_status; } //!< The result of the operation
        RequestPurpose Purpose() const { return m_purpose; } //!< The purpose of the query which generated this response
        ResponseOptions Options() const { return m_options; } //!< The options which were specified with the request, signifying the details included in this response
        void SetOptions(ResponseOptions options) { m_options = options; } //!< Change the response options
        void SetPurpose(RequestPurpose purpose) { m_purpose = purpose; } //!< Change the request purpose

        //! Returns whether this response was obtained from the server. If not, full ownership details may not be included in LockStates() or CodeStates() and should be obtained from the server instead.
        bool WasServerQuery() const { return RequestPurpose::FastQuery != m_purpose; }

        DgnLockInfoSet const& LockStates() const { return m_lockStates; } //!< The states of any locks which could not be acquired, if ResponseOptions::LockState was specified
        DgnCodeInfoSet const& CodeStates() const { return m_codeStates; } //!< The states of any codes which could not be reserved, if ResponseOptions::CodeState was specified

        void SetResult(RepositoryStatus result) { m_status = result; } //!< Set the result of the operation
        DgnLockInfoSet& LockStates() { return m_lockStates; } //!< A writable reference to the lock states
        DgnCodeInfoSet& CodeStates() { return m_codeStates; } //!< A writable reference to the code states

        //! Reset to default state ("invalid response") while preserving the ResponseOptions and RequestPurpose
        void Invalidate() { m_status = RepositoryStatus::InvalidResponse; m_lockStates.clear(); m_codeStates.clear(); }

        DGNPLATFORM_EXPORT void ToJson(JsonValueR value) const; //!< Convert to JSON representation
        DGNPLATFORM_EXPORT bool FromJson(JsonValueCR value); //!< Attempt to initialize from JSON representation
    };

    enum class ChannelType { None, Shared, Normal };

    struct ChannelProps
        {
        ChannelType channelType = ChannelType::None;
        bool isInitializingChannel {};
        bool reportCodesInLockedModels = true;
        bool includeUsedLocksInChangeSet = true;
        bool stayInChannel = false;
        bool oneBriefcaseOwnsChannel = true;
        DgnElementId channelParentId;
        };

    //! Options for querying availability of locks and codes
    enum class FastQuery
    {
        No, //!< Query the server. May be slow, but results will be up-to-date.
        Yes //!< Query a local cache. Faster, but results may be out-of-date with server state.
    };
private:
    DgnDbR  m_db;
protected:
    ChannelProps m_channelProps;

private:
    void ReformulateLockRequest(LockRequestR, Response const&) const;
    void ReformulateCodeRequest(DgnCodeSet&, Response const&) const;
    void RemoveElements(LockRequestR, DgnModelId) const;
protected:
    explicit IBriefcaseManager(DgnDbR db) : m_db(db) { }

    // Codes and Locks
    virtual Response _ProcessRequest(Request& request, RequestPurpose purpose) = 0;
    virtual RepositoryStatus _Demote(DgnLockSet& locks, DgnCodeSet const& codes) = 0;
    virtual RepositoryStatus _Relinquish(Resources which) = 0;
    virtual bool _AreResourcesHeld(DgnLockSet& locks, DgnCodeSet& codes, RepositoryStatus* status) = 0;
    virtual RepositoryStatus _PrepareForElementOperation(Request& req, DgnElementCR el, BeSQLite::DbOpcode opcode) = 0;
    virtual RepositoryStatus _PrepareForModelOperation(Request& req, DgnModelCR model, BeSQLite::DbOpcode opcode) = 0;

    // Codes
    virtual RepositoryStatus _QueryCodeStates(DgnCodeInfoSet& states, DgnCodeSet const& codes) = 0;
    DGNPLATFORM_EXPORT virtual RepositoryStatus _ReserveCode(DgnCodeCR code);

    // Locks
    virtual RepositoryStatus _QueryLockLevel(LockLevel& level, LockableId lockId) = 0;
    virtual RepositoryStatus _QueryLockLevels(DgnLockSet& lockLevels, LockableIdSet& lockIds) = 0;
    virtual IOwnedLocksIteratorPtr _GetOwnedLocks(FastQuery fast) = 0;

    // Local state management
    virtual RepositoryStatus _OnFinishRevision(DgnRevision const& rev) = 0;
    virtual void _OnElementInserted(DgnElementId id) = 0;
    virtual void _OnModelInserted(DgnModelId id) = 0;
    virtual RepositoryStatus _RefreshFromRepository() = 0;
    virtual RepositoryStatus _ClearUserHeldCodesLocks() = 0;
    virtual void _OnDgnDbDestroyed() { }

    // Bulk operations
    virtual void _StartBulkOperation() = 0;
    virtual bool _IsBulkOperation() const = 0;
    virtual Response _EndBulkOperation() = 0;
    virtual void _ExtractRequestFromBulkOperation(Request&, bool locks, bool codes) {;}
    virtual bset<CodeSpecId> _GetFilteredCodeSpecIds() { return bset<CodeSpecId>(); }

    // Channel management
    virtual DgnElementId _GetNormalChannelParentOf(DgnModelId mid, DgnElementCP el) {return DgnElementId();}
    virtual bool _IsLockRequired(DgnElementCR element) {return true;}
    DGNPLATFORM_EXPORT virtual Response _LockChannelParent();

    DGNPLATFORM_EXPORT virtual RepositoryStatus _PerformPrepareAction(Request& req, PrepareAction action);

    DGNPLATFORM_EXPORT IRepositoryManagerP GetRepositoryManager() const;
    DGNPLATFORM_EXPORT bool LocksRequired() const;
    DGNPLATFORM_EXPORT RepositoryStatus PrepareForElementOperation(Request& req, DgnElementCR el, BeSQLite::DbOpcode opcode, PrepareAction action);
    DGNPLATFORM_EXPORT RepositoryStatus PrepareForModelOperation(Request& req, DgnModelCR model, BeSQLite::DbOpcode opcode, PrepareAction action);
    RepositoryStatus PerformPrepareAction(Request& req, PrepareAction action);
    DgnDbStatus OnElementOperation(DgnElementCR el, BeSQLite::DbOpcode opcode);
    DgnDbStatus OnModelOperation(DgnModelCR model, BeSQLite::DbOpcode opcode);
    static DgnDbStatus ToDgnDbStatus(RepositoryStatus repoStatus, Request const& request);
public:
    DgnDbR GetDgnDb() const { return m_db; } //!< The DgnDb managed by this object

    Response LockChannelParent() {return _LockChannelParent();}

    //! Of this model is in a *normal channel* return its channel parent ID.
    //! @param mid The ModelId of the model to check
    //! @param el  If available, the element in the model that is being accessed.
    //! @return the channel parent's DgnElementId if in a normal channel or an invalid ID if not.
    DgnElementId GetNormalChannelParentOf(DgnModelId mid, DgnElementCP el) {return _GetNormalChannelParentOf(mid, el);}

    //! Is the specified element in a model that is in a normal channel?
    DGNPLATFORM_EXPORT DgnElementId GetNormalChannelParentOf(DgnElementCR el);

    //! Is a lock required before writing to the specified element?
    bool IsLockRequired(DgnElementCR element) {return _IsLockRequired(element);}

    bool IsNoChannel() const {return ChannelType::None == m_channelProps.channelType;}
    bool IsSharedChannel() const {return ChannelType::Shared == m_channelProps.channelType;}
    bool IsNormalChannel() const {return ChannelType::Normal == m_channelProps.channelType;}

    // Should the changeset be examined to determine what locks are needed before pushing?
    bool ShouldIncludeUsedLocksInChangeSet() {return m_channelProps.includeUsedLocksInChangeSet;}
    // Should the Codes found on elements in exclusively locked models be reported as used to the Code service?
    bool ShouldReportCodesInLockedModels() {return m_channelProps.reportCodesInLockedModels;}
    // Should client code be restricted to write to shared definition models *only* in the Shared channel and to bridge models *only* in the Normal channel
    bool StayInChannel() {return m_channelProps.stayInChannel;}

    ChannelProps GetChannelProps() const {return m_channelProps;}
    ChannelProps& GetChannelPropsR() {return m_channelProps;}
    void SetChannelProps(ChannelProps const& p) {m_channelProps=p;}

    //! @name Managing both Locks and Codes
    //@{
    //! In normal mode, process the request and return a response, forwarding the request to the repository manager if required.
    //! In bulk operation mode, this function does not forward the request to the repository manager but merely adds it to the pending request.
    //! @param[in]      request The set of locks and/or codes to acquire, and options for customizing the response
    //! @return The response, containing the result and any additional details as specified by the request's ResponseOptions
    //! @remarks Note the contents of the request may be modified.
    Response Acquire(Request& request) { return _ProcessRequest(request, RequestPurpose::Acquire); }

    //! Query whether all of a set of locks and codes are available for acquisition by this briefcase.
    //! @param[in]      request  The set of locks and/or codes to acquire, and options for customizing the response
    //! @param[out]     response If supplied, populated with the results of the operation
    //! @param[in]      fast     Whether to query the repository manager directly (slow but accurate) or local cache (fast but less accurate)
    //! @return True if all locks and codes are available; LockAlreadyHeld or CodeUnavailable if some are not; or an error status if the operation failed.
    //! @remarks Note the contents of the request may be modified.
    DGNPLATFORM_EXPORT bool AreResourcesAvailable(Request& request, Response* response=nullptr, FastQuery fast=FastQuery::No);

    //! Demote the previously-acquired locks to the specified levels, and release the previously-reserved codes
    //! In bulk operation mode, this function also updates the pending request.
    //! @param[in]      locks The set of locks to demote and their new levels
    //! @param[in]      codes The set of codes to release
    //! @return Success, or an error code
    //! @remarks This method cannot be used to increase a lock's level. It cannot be used to release locks or codes required by the briefcase's local changes.
    RepositoryStatus Demote(DgnLockSet& locks, DgnCodeSet const& codes) { return _Demote(locks, codes); }

    //! Relinquish all reserved codes and/or all held locks
    //! @param[in]      which The type(s) of resources to relinquish
    //! @return Success, or an error status
    //! @remarks This method cannot be used to release locks or codes required by the briefcase's local changes.
    //! @note This function should not be called in bulk operation mode.
    RepositoryStatus Relinquish(Resources which=Resources::All) { return _Relinquish(which); }

    //! Returns whether the specified resources are held by this briefcase.
    //! @param[in]      locks  A set of locks and levels
    //! @param[in]      codes  A set of codes
    //! @param[in]      status If non-null, receives the result of the operation
    //! @return This function returns true if the operation completed successfully, all of the locks are held at or above the desired level, and all of the codes are reserved.
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

    //! Convenience function to acquire all locks and codes required for element insertion. Prefer batch operations instead where possible.
    //! In bulk operation mode, this function does not acquire the resources but merely adds them to the pending request.
    RepositoryStatus AcquireForElementInsert(DgnElementCR el) { Request req; return PrepareForElementInsert(req, el, PrepareAction::Acquire); }

    //! Convenience function to acquire all locks and codes required for element update. Prefer batch operations instead where possible.
    //! In bulk operation mode, this function does not acquire the resources but merely adds them to the pending request.
    RepositoryStatus AcquireForElementUpdate(DgnElementCR el) { Request req; return PrepareForElementUpdate(req, el, PrepareAction::Acquire); }

    //! Convenience function to acquire all locks and codes required for element deletion. Prefer batch operations instead where possible.
    //! In bulk operation mode, this function does not acquire the resources but merely adds them to the pending request.
    RepositoryStatus AcquireForElementDelete(DgnElementCR el) { Request req; return PrepareForElementDelete(req, el, PrepareAction::Acquire); }

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
    //! @note This method should not be called while in bulk operation mode.
    RepositoryStatus RelinquishLocks() { return _Relinquish(Resources::Locks); }

    //! Reduces the levels at which a set of locks are held
    //! In bulk operation mode, this function also updates the pending request.
    //! @param[in]      locks The set of locks and their new levels
    //! @return Success, or an error status
    //! @remarks This method cannot be used to increase a lock's level or to release locks required by the briefcase's local changes. The DgnLockSet may be modified
    RepositoryStatus DemoteLocks(DgnLockSet& locks) { return _Demote(locks, DgnCodeSet()); }

    //! Attempts to acquire a set of locks
    //! In bulk operation mode, this function does not acquire the resources but merely adds them to the pending request.
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
    //! @note This function should not be called while in bulk operation mode.
    DGNPLATFORM_EXPORT void ReformulateRequest(Request& req, Response const& response) const;

    //! Acquire a shared or exclusive lock on the DgnDb.
    //! In bulk operation mode, this function does not acquire the resource but merely adds it to the pending request.
    Response LockDb(LockLevel level) { Request req; req.Locks().Insert(GetDgnDb(), level); return Acquire(req); }

    //! Acquire an exclusive lock on Schemas in the DgnDb. 
    //! In bulk operation mode, this function does not acquire the resource but merely adds it to the pending request.
    Response LockSchemas() { Request req; req.SetOptions(ResponseOptions::All); req.Locks().InsertSchemasLock(GetDgnDb()); return Acquire(req); }

    //! Acquire an exclusive lock on CodeSpecs in the DgnDb. 
    //! In bulk operation mode, this function does not acquire the resource but merely adds it to the pending request.
    Response LockCodeSpecs() { Request req; req.SetOptions(ResponseOptions::All); req.Locks().InsertCodeSpecsLock(GetDgnDb()); return Acquire(req); }

    //! Obtain an iterator over the locks currently owned by this briefcase.
    //! By default, iteration is provided over a local cache of held locks. Specifying FastQuery::Yes will query the server for an up-to-date list of held locks.
    //! @param[in] fast Whether to iterate over local cache of held locks, or to query server for up-to-date information.
    //! @return An iterator over the locks currently owned by this briefcase, or nullptr if such an iterator cannot be created.
    IOwnedLocksIteratorPtr GetOwnedLocks(FastQuery fast=FastQuery::Yes) { return _GetOwnedLocks(fast); }
    //@}

    //! @name Managing Codes
    //@{
    //! Query the states of a set of codes
    //! @param[in]      states Upon successful return, holds the state of each code
    //! @param[in]      codes  The set of codes to query
    //! @return Success, or an error status
    RepositoryStatus QueryCodeStates(DgnCodeInfoSet& states, DgnCodeSet const& codes) { return _QueryCodeStates(states, codes); }

    //! Attempts to reserve a single code for use by this briefcase
    //! In bulk operation mode, this function does not acquire the resource but merely adds it to the pending request.
    //! @param[in]      code The code to reserve
    //! @return Success if the code was reserved, or an error status
    RepositoryStatus ReserveCode(DgnCodeCR code) { return _ReserveCode(code); }

    //! Relinquishes all codes reserved by this briefcase
    //! In bulk operation mode, this function also updates the pending request.
    //! @return Success, or an error status
    //! @remarks This method cannot be used to release codes required by the briefcase's local changes
    RepositoryStatus RelinquishCodes() { return _Relinquish(Resources::Codes); }

    //! Release a set of codes reserved by this briefcase
    //! In bulk operation mode, this function also updates the pending request.
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

    //! Returns a set of the filtered CodeSpecIds that we don't want to include in bulk requests
    //! @return set of CodeSpecId
    bset<CodeSpecId> GetFilteredCodeSpecIds() { return _GetFilteredCodeSpecIds(); }

    //! Attempts to reserve a set of codes for this briefcase's use
    //! In bulk operation mode, this function does not acquire the resource but merely adds it to the pending request.
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
    //! Clears user held locks from local cache, only used in special workflows
    RepositoryStatus ClearUserHeldCodesLocks() { return _ClearUserHeldCodesLocks(); }
    RepositoryStatus OnFinishRevision(DgnRevision const& rev) { return _OnFinishRevision(rev); } //!< @private
    void OnElementInserted(DgnElementId id); //!< @private
    void OnModelInserted(DgnModelId id); //!< @private
    //@}

    //! @name Bulk Operations
    //! @{
    //! Call this before starting a group of changes that will require locks and/or codes.
    //! Call DgnDb::SaveChanges to end bulk operation mode and acquire locks and codes. Or, you can call EndBulkOperation directly.
    void StartBulkOperation() {_StartBulkOperation();}
    
    //! Check if a bulk operation is in progress
    bool IsBulkOperation() const {return _IsBulkOperation();}

    void ExtractRequestFromBulkOperation(Request& r, bool locks, bool codes) {_ExtractRequestFromBulkOperation(r, locks, codes);}

    //! Call this if you want to acquire locks and codes @em before the end of the transaction.
    //! @note DgnDb::SaveChanges automatically calls this function to end the bulk operation and acquire locks and codes.
    //! If successful, this terminates the bulk operation.
    //! If not successful, the caller should abandon all changes.
    //! @note This must be called on the same thread that called StartBulkOperation.
    IBriefcaseManager::Response EndBulkOperation() {return _EndBulkOperation();}
    //! @}

    DgnDbStatus OnElementInsert(DgnElementCR el); //!< @private
    DgnDbStatus OnElementUpdate(DgnElementCR el); //!< @private
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
    virtual Response _ProcessRequest(Request const& req, DgnDbR db, bool queryOnly) = 0;
    virtual RepositoryStatus _Demote(DgnLockSet const& locks, DgnCodeSet const& codes, DgnDbR db) = 0;
    virtual RepositoryStatus _Relinquish(Resources which, DgnDbR db) = 0;
    virtual RepositoryStatus _QueryHeldResources(DgnLockSet& locks, DgnCodeSet& codes, DgnLockSet& unavailableLocks, DgnCodeSet& unavailableCodes, DgnDbR db) = 0;
    virtual RepositoryStatus _QueryStates(DgnLockInfoSet& lockStates, DgnCodeInfoSet& codeStates, LockableIdSet const& locks, DgnCodeSet const& codes) = 0;
    DGNPLATFORM_EXPORT virtual RepositoryStatus _QueryHeldLocks(DgnLockSet&, DgnDbR);

public:
    //! Process a briefcase's request to acquire locks and/or reserve codes
    //! @param[in]      req The set of desired codes and/or locks, and options for customizing what data is included in the response
    //! @param[in]      db  The requesting briefcase
    //! @return The response, customized according to the specified options
    Response Acquire(Request const& req, DgnDbR db) { return _ProcessRequest(req, db, false); }

    //! Query whether a briefcase's request to acquire locks and/or reserve codes could be fulfilled, without actually fulfulling it
    //! @param[in]      req The set of desired codes and/or locks, and options for customizing what data is included in the response
    //! @param[in]      db  The requesting briefcase
    //! @return The response, customized according to the specified options
    Response QueryAvailability(Request const& req, DgnDbR db) { return _ProcessRequest(req, db, true); }

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
    //! @param[out]     locks The set of locks tracked by the repository and held by the briefcase
    //! @param[out]     codes The set of codes tracked by the repository and held by the briefcase
    //! @param[out]     unavailableLocks The set of locks tracked by the repository and unavailable for acquisition by this briefcase
    //! @param[out]     unavailableCodes The set of codes tracked by the repository and unavailable for acquisition by this briefcase
    //! @param[in]      db    The requesting briefcase
    //! @return Success, or an error status
    //! @remarks This method only returns resources tracked by the repository - e.g., excluding locks implicitly held for elements/models created locally by this briefcase and not yet committed to the repository
    RepositoryStatus QueryHeldResources(DgnLockSet& locks, DgnCodeSet& codes, DgnLockSet& unavailableLocks, DgnCodeSet& unavailableCodes, DgnDbR db) { return _QueryHeldResources(locks, codes, unavailableLocks, unavailableCodes, db); }

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

    //! Returns all locks held by the briefcase.
    //! @param[in]      states Upon successful return, holds the state of each lock held by the briefcase
    //! @param[in]      db    The requesting briefcase
    //! @return Success, or an error status
    //! @remarks This method only returns locks tracked by the repository - e.g., excluding locks implicitly held for elements/models created locally by this briefcase and not yet committed to the repository
    DGNPLATFORM_EXPORT RepositoryStatus QueryHeldLocks(DgnLockSet& states, DgnDbR db) { return _QueryHeldLocks(states, db); };

    //! Get a readable description of a RepositoryStatus
    DGNPLATFORM_EXPORT static Utf8String RepositoryStatusToString(RepositoryStatus);

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
    DGNPLATFORM_EXPORT bool LockLevelFromUInt(LockLevel& level, unsigned int value);
    DGNPLATFORM_EXPORT bool LockableTypeFromUInt(LockableType& type, unsigned int value);
    DGNPLATFORM_EXPORT bool RepositoryStatusFromUInt(RepositoryStatus& status, unsigned int value);
    DGNPLATFORM_EXPORT bool ResponseOptionsFromUInt(IBriefcaseManager::ResponseOptions& options, unsigned int value);
    DGNPLATFORM_EXPORT bool RequestPurposeFromUInt(IBriefcaseManager::RequestPurpose& purpose, unsigned int value);

    DGNPLATFORM_EXPORT bool BriefcaseIdFromJson(BeSQLite::BeBriefcaseId& id, JsonValueCR value);
    DGNPLATFORM_EXPORT bool BeInt64IdFromJson(BeInt64Id& id, JsonValueCR value);
    DGNPLATFORM_EXPORT bool LockLevelFromJson(LockLevel& level, JsonValueCR value);
    DGNPLATFORM_EXPORT bool LockableTypeFromJson(LockableType& type, JsonValueCR value);
    DGNPLATFORM_EXPORT bool RepositoryStatusFromJson(RepositoryStatus& status, JsonValueCR value);
    DGNPLATFORM_EXPORT bool ResponseOptionsFromJson(IBriefcaseManager::ResponseOptions& options, JsonValueCR value);
    DGNPLATFORM_EXPORT bool RequestPurposeFromJson(IBriefcaseManager::RequestPurpose& purpose, JsonValueCR value);

    DGNPLATFORM_EXPORT void BriefcaseIdToJson(JsonValueR value, BeSQLite::BeBriefcaseId id);
    DGNPLATFORM_EXPORT void BeInt64IdToJson(JsonValueR value, BeInt64Id id);
    DGNPLATFORM_EXPORT void LockLevelToJson(JsonValueR value, LockLevel level);
    DGNPLATFORM_EXPORT void LockableTypeToJson(JsonValueR value, LockableType type);
    DGNPLATFORM_EXPORT void RepositoryStatusToJson(JsonValueR value, RepositoryStatus status);
    DGNPLATFORM_EXPORT void ResponseOptionsToJson(JsonValueR value, IBriefcaseManager::ResponseOptions options);
    DGNPLATFORM_EXPORT void RequestPurposeToJson(JsonValueR value, IBriefcaseManager::RequestPurpose purpose);
} // namespace RepositoryJson
//__PUBLISH_SECTION_START__

struct IOptimisticConcurrencyControl;

/* A concurrency control policy */
struct IConcurrencyControl : IRefCounted
    {
    virtual IOptimisticConcurrencyControl* _AsIOptimisticConcurrencyControl() = 0;
    virtual void _ConfigureBriefcaseManager(IBriefcaseManager&) = 0;
    virtual void _OnProcessRequest(IBriefcaseManager::Request&, IBriefcaseManager&, IBriefcaseManager::RequestPurpose) = 0;
    virtual void _OnProcessedRequest(IBriefcaseManager::Request&, IBriefcaseManager&, IBriefcaseManager::RequestPurpose, IBriefcaseManager::Response&) = 0;
    virtual void _OnQueryHeld(DgnLockSet&, DgnCodeSet&, IBriefcaseManager&) = 0;
    virtual void _OnQueriedHeld(DgnLockSet&, DgnCodeSet&, IBriefcaseManager&) = 0;
    virtual void _OnExtractRequest(IBriefcaseManager::Request&, IBriefcaseManager&) = 0;
    virtual void _OnExtractedRequest(IBriefcaseManager::Request&, IBriefcaseManager&) = 0;
    };

/* The pessimistic concurrency control policy. Locks and codes must be acquired before a changeset can be pushed to iModelHub. */
struct PessimisticConcurrencyControl : RefCounted<IConcurrencyControl>
    {
    IOptimisticConcurrencyControl* _AsIOptimisticConcurrencyControl() override {return nullptr;}
    void _ConfigureBriefcaseManager(IBriefcaseManager&) override {}
    void _OnProcessRequest(IBriefcaseManager::Request&, IBriefcaseManager&, IBriefcaseManager::RequestPurpose) override {}
    void _OnProcessedRequest(IBriefcaseManager::Request&, IBriefcaseManager&, IBriefcaseManager::RequestPurpose, IBriefcaseManager::Response&) override {}
    void _OnQueryHeld(DgnLockSet&, DgnCodeSet&, IBriefcaseManager&) override {}
    void _OnQueriedHeld(DgnLockSet&, DgnCodeSet&, IBriefcaseManager&) override {}
    void _OnExtractRequest(IBriefcaseManager::Request&, IBriefcaseManager&) override {}
    void _OnExtractedRequest(IBriefcaseManager::Request&, IBriefcaseManager&) override {}
    };

/* An optimistic concurrency control policy */
struct IOptimisticConcurrencyControl : IConcurrencyControl
    {
    virtual BeSQLite::ChangeSet::ConflictResolution _OnConflict(DgnDbCR, BeSQLite::ChangeSet::ConflictCause, BeSQLite::Changes::Change) = 0;

    virtual bset<DgnElementId> const& _GetConflictingElementsRejected() const = 0;
    virtual bset<DgnElementId> const& _GetConflictingElementsAccepted() const = 0;
    virtual void _ConflictsProcessed() = 0;
    };

/* An optimistic concurrency control policy */
struct OptimisticConcurrencyControlBase : IOptimisticConcurrencyControl
    {
    DgnLockSet m_locks;
    DgnLockSet m_locksTemp;

    IOptimisticConcurrencyControl* _AsIOptimisticConcurrencyControl() override {return this;}
    DGNPLATFORM_EXPORT void _ConfigureBriefcaseManager(IBriefcaseManager&) override;
    DGNPLATFORM_EXPORT void _OnProcessRequest(IBriefcaseManager::Request&, IBriefcaseManager&, IBriefcaseManager::RequestPurpose) override;
    DGNPLATFORM_EXPORT void _OnProcessedRequest(IBriefcaseManager::Request&, IBriefcaseManager&, IBriefcaseManager::RequestPurpose, IBriefcaseManager::Response&) override;
    DGNPLATFORM_EXPORT void _OnQueryHeld(DgnLockSet&, DgnCodeSet&, IBriefcaseManager&) override;
    DGNPLATFORM_EXPORT void _OnQueriedHeld(DgnLockSet&, DgnCodeSet&, IBriefcaseManager&) override;
    DGNPLATFORM_EXPORT void _OnExtractRequest(IBriefcaseManager::Request&, IBriefcaseManager&) override;
    DGNPLATFORM_EXPORT void _OnExtractedRequest(IBriefcaseManager::Request&, IBriefcaseManager&) override;
    };

/* An optimistic concurrency control policy where conflict-resolution is controlled by policy settings. */
struct OptimisticConcurrencyControl : RefCounted<OptimisticConcurrencyControlBase>
    {
    /** How to handle a conflict */
    enum class OnConflict
        {
        /** Reject the incoming change */
        RejectIncomingChange = 0,
        /** Accept the incoming change */
        AcceptIncomingChange = 1,
        };
        
    /** The options for how conflicts are to be handled during change-merging in an OptimisticConcurrencyControlPolicy.
     * The scenario is that the caller has made some changes to the *local* briefcase. Now, the caller is attempting to
     * merge in changes from iModelHub. The properties of this policy specify how to handle the *incoming* changes from iModelHub.
     */
    struct Policy
        {
        /** What to do with the incoming change in the case where the same entity was updated locally and also would be updated by the incoming change. */
        OnConflict updateVsUpdate;
        /** What to do with the incoming change in the case where an entity was updated locally and would be deleted by the incoming change. */
        OnConflict updateVsDelete;
        /** What to do with the incoming change in the case where an entity was deleted locally and would be updated by the incoming change. */
        OnConflict deleteVsUpdate;
        };

    private:
    Policy m_policy;
    bset<DgnElementId> m_conflictingElementsRejected;
    bset<DgnElementId> m_conflictingElementsAccepted;

    BeSQLite::ChangeSet::ConflictResolution HandleConflict(OptimisticConcurrencyControl::OnConflict, DgnDbCR, Utf8CP tableName, BeSQLite::Changes::Change, BeSQLite::DbOpcode, bool indirect);

    public:
    DGNPLATFORM_EXPORT OptimisticConcurrencyControl(Policy conflicts);

    BeSQLite::ChangeSet::ConflictResolution _OnConflict(DgnDbCR, BeSQLite::ChangeSet::ConflictCause, BeSQLite::Changes::Change) override;
    bset<DgnElementId> const& _GetConflictingElementsRejected() const override {return m_conflictingElementsRejected;}
    bset<DgnElementId> const& _GetConflictingElementsAccepted() const override {return m_conflictingElementsAccepted;}
    void _ConflictsProcessed() override {m_conflictingElementsRejected.clear(); m_conflictingElementsAccepted.clear();}
    };



END_BENTLEY_DGNPLATFORM_NAMESPACE


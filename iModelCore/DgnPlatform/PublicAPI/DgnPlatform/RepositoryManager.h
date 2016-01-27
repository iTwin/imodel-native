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
//! 
// @bsistruct                                                    Paul.Connelly   01/16
/*=====================================================================================*/
struct IBriefcaseManager : RefCountedBase
{
public:
    enum class Resources : uint8_t
    {
        None = 0,
        Locks = 1 << 0,
        Codes = 1 << 1,
        All = 0xff
    };

    enum class ResponseOptions : uint8_t
    {
        None = 0, //!< No special options
        DeniedLocks = 1 << 0, //!< If a request to acquire locks is denied, the response will include the current lock state of each denied lock
        CodeState = 1 << 1, //!< Include DgnCodeState for any codes for which the request was denied
        All = 0xff, //!< Include all options
    };

    struct Request
    {
    private:
        LockRequest     m_locks;
        DgnCodeSet      m_codes;
        ResponseOptions m_options;
    public:
        explicit Request(ResponseOptions opts=ResponseOptions::None) : m_options(opts) { }

        LockRequest const& Locks() const { return m_locks; }
        DgnCodeSet const& Codes() const { return m_codes; }
        ResponseOptions Options() const { return m_options; }
        bool IsEmpty() const { return m_locks.IsEmpty() && m_codes.empty(); }

        LockRequest& Locks() { return m_locks; }
        DgnCodeSet& Codes() { return m_codes; }
        void SetOptions(ResponseOptions opts) { m_options = opts; }
    };

    struct Response
    {
    private:
        DgnCodeInfoSet      m_codeStates;
        DgnLockSet          m_deniedLocks;
        RepositoryStatus    m_status;
    public:
        explicit Response(RepositoryStatus status=RepositoryStatus::InvalidResponse) : m_status(status) { }

        RepositoryStatus Result() const { return m_status; }
        DgnLockSet const& DeniedLocks() const { return m_deniedLocks; }
        DgnCodeInfoSet const& CodeStates() const { return m_codeStates; }

        void SetResult(RepositoryStatus result) { m_status = result; }
        DgnLockSet& DeniedLocks() { return m_deniedLocks; }
        DgnCodeInfoSet& CodeStates() { return m_codeStates; }

        void Invalidate() { m_status = RepositoryStatus::InvalidResponse; m_deniedLocks.clear(); m_codeStates.clear(); }
    };
        
private:
    DgnDbR  m_db;
protected:
    explicit IBriefcaseManager(DgnDbR db) : m_db(db) { }

    // Codes and Locks
    virtual Response _ProcessRequest(Request& request) = 0;
    virtual RepositoryStatus _Demote(DgnLockSet& locks, DgnCodeSet const& codes) = 0;
    virtual RepositoryStatus _Relinquish(Resources which) = 0;
    virtual bool _AreResourcesHeld(DgnLockSet& locks, DgnCodeSet& codes, RepositoryStatus* status) = 0;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _LockElement(DgnElementCR el, DgnCodeCR code, DgnModelId originalModelId);
    DGNPLATFORM_EXPORT virtual DgnDbStatus _LockModel(DgnModelCR model, LockLevel level, DgnCodeCR code);
    DGNPLATFORM_EXPORT virtual DgnDbStatus _LockDb(LockLevel level, DgnCodeCR code);
    DgnDbStatus LockElement(DgnElementCR el, DgnCodeCR code, DgnModelId originalModelId=DgnModelId()) { return _LockElement(el, code, originalModelId); }

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

    DGNPLATFORM_EXPORT IRepositoryManagerP GetRepositoryManager() const;
public:
    DgnDbR GetDgnDb() const { return m_db; }

    //! @name Managing both Locks and Codes
    //@{
    Response ProcessRequest(Request& request) { return _ProcessRequest(request); }

    RepositoryStatus Demote(DgnLockSet& locks, DgnCodeSet const& codes) { return _Demote(locks, codes); }

    RepositoryStatus Relinquish(Resources which=Resources::All) { return _Relinquish(which); }

    bool AreResourcesHeld(DgnLockSet& locks, DgnCodeSet& codes, RepositoryStatus* status=nullptr) { return _AreResourcesHeld(locks, codes, status); }
    //@}

    //! @name Managing Locks
    //{
    bool AreLocksHeld(DgnLockSet& locks, RepositoryStatus* status=nullptr) { DgnCodeSet codes; return AreResourcesHeld(locks, codes, status); }

    RepositoryStatus RelinquishLocks() { return _Relinquish(Resources::Locks); }

    RepositoryStatus DemoteLocks(DgnLockSet& locks) { return _Demote(locks, DgnCodeSet()); }

    DGNPLATFORM_EXPORT Response AcquireLocks(LockRequestR locks, ResponseOptions options=ResponseOptions::None);

    RepositoryStatus QueryLockLevel(LockLevel& level, LockableId lockId) { return _QueryLockLevel(level, lockId); }

    RepositoryStatus QueryLockLevels(DgnLockSet& lockLevels, LockableIdSet& lockIds) { return _QueryLockLevels(lockLevels, lockIds); }

    LockLevel QueryLockLevel(LockableId lockId) { LockLevel level; return RepositoryStatus::Success == QueryLockLevel(level, lockId) ? level : LockLevel::None; }
    LockLevel QueryLockLevel(DgnDbCR db) { return QueryLockLevel(LockableId(db)); }
    LockLevel QueryLockLevel(DgnElementCR el) { return QueryLockLevel(LockableId(el)); }
    LockLevel QueryLockLevel(DgnModelCR model) { return QueryLockLevel(LockableId(model)); }

    //! Reformulate a denied request such that it does not contain any of the locks in the "denied" set.
    //! If the request contains locks which are dependent upon other locks in the denied set (e.g., elements within a model for which the model lock was not granted),
    //! the dependent locks will be removed.
    DGNPLATFORM_EXPORT void ReformulateRequest(Request& req, Response const& response) const;
    //@}

    //! @name Managing Codes
    //@{
    RepositoryStatus QueryCodeStates(DgnCodeInfoSet& states, DgnCodeSet const& codes) { return _QueryCodeStates(states, codes); }

    RepositoryStatus ReserveCode(DgnCodeCR code) { return _ReserveCode(code); }

    RepositoryStatus RelinquishCodes() { return _Relinquish(Resources::Codes); }

    RepositoryStatus ReleaseCodes(DgnCodeSet const& codes) { DgnLockSet locks; return _Demote(locks, codes); }

    bool AreCodesReserved(DgnCodeSet& codes, RepositoryStatus* status=nullptr) { DgnLockSet locks; return AreResourcesHeld(locks, codes, status); }

    DGNPLATFORM_EXPORT Response ReserveCodes(DgnCodeSet& codes, ResponseOptions options=ResponseOptions::None);
    //@}

    //! @name Local State Management
    //@{
    RepositoryStatus OnFinishRevision(DgnRevision const& rev) { return _OnFinishRevision(rev); }
    void OnElementInserted(DgnElementId id); //!< @private
    void OnModelInserted(DgnModelId id); //!< @private
    //@}

    DgnDbStatus OnElementInsert(DgnElementCR el); //!< @private
    DgnDbStatus OnElementUpdate(DgnElementCR el, DgnModelId originalModelId); //!< @private
    DgnDbStatus OnElementDelete(DgnElementCR el); //!< @private
    DgnDbStatus OnModelInsert(DgnModelCR model); //!< @private
    DgnDbStatus OnModelUpdate(DgnModelCR model); //!< @private
    DgnDbStatus OnModelDelete(DgnModelCR model); //!< @private
//__PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT static void BackDoor_SetEnabled(bool enable);
//__PUBLISH_SECTION_START__
};

/*=====================================================================================*/
//!
// @bsistruct                                                    Paul.Connelly   01/16
/*=====================================================================================*/
struct IRepositoryManager
{
    typedef IBriefcaseManager::Request Request;
    typedef IBriefcaseManager::Response Response;
    typedef IBriefcaseManager::Resources Resources;
protected:
    // Codes + Locks
    virtual Response _ProcessRequest(Request const& req, DgnDbR db) = 0;
    virtual RepositoryStatus _Demote(DgnLockSet const& locks, DgnCodeSet const& codes, DgnDbR db) = 0;
    virtual RepositoryStatus _Relinquish(Resources which, DgnDbR db) = 0;
    virtual RepositoryStatus _QueryHeldResources(DgnLockSet& locks, DgnCodeSet& codes, DgnDbR db) = 0;
    virtual RepositoryStatus _QueryStates(DgnOwnedLockSet& ownership, DgnCodeInfoSet& codeStates, LockableIdSet const& locks, DgnCodeSet const& codes) = 0;
public:
    Response ProcessRequest(Request const& req, DgnDbR db) { return _ProcessRequest(req, db); }
    RepositoryStatus Demote(DgnLockSet const& locks, DgnCodeSet const& codes, DgnDbR db) { return _Demote(locks, codes, db); }
    RepositoryStatus Relinquish(Resources which, DgnDbR db) { return _Relinquish(which, db); }
    RepositoryStatus QueryStates(DgnOwnedLockSet& lockOwnership, DgnCodeInfoSet& codeStates, LockableIdSet const& locks, DgnCodeSet const& codes) { return _QueryStates(lockOwnership, codeStates, locks, codes); }
    RepositoryStatus QueryHeldResources(DgnLockSet& locks, DgnCodeSet& codes, DgnDbR db) { return _QueryHeldResources(locks, codes, db); }

    DGNPLATFORM_EXPORT RepositoryStatus QueryCodeStates(DgnCodeInfoSet& states, DgnCodeSet const& codes);
    DGNPLATFORM_EXPORT RepositoryStatus QueryLockOwnerships(DgnOwnedLockSet& ownership, LockableIdSet const& locks);
};

ENUM_IS_FLAGS(IBriefcaseManager::ResponseOptions);
ENUM_IS_FLAGS(IBriefcaseManager::Resources);

END_BENTLEY_DGNPLATFORM_NAMESPACE


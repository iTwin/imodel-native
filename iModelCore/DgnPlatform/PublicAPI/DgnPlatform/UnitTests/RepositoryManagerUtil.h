
/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once 

// __BENTLEY_INTERNAL_ONLY__

#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnCoreAPI.h>
#include <DgnPlatform/GenericDomain.h>

#define TABLE_Locks "Locks"
#define LOCK_Id "Id"
#define LOCK_Type "Type"
#define LOCK_BcId "Owner"
#define LOCK_Exclusive "Exclusive"

#define TABLE_Codes "Codes"
#define CODE_CodeSpec "CodeSpec"
#define CODE_Scope "Scope"
#define CODE_Value "Name"
#define CODE_State "State"
#define CODE_Revision "Revision"
#define CODE_Briefcase "Briefcase"

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_BENTLEY_DGN_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* There used to be two separate interfaces: one for locks, another for codes.
* We combined them.
* Minimal changes made to this mock implementation - mostly just performs lock+code
* operations independently.
* That mostly works because we generally also test locks+codes independently.
* A real implementation would need to take care that if an operation involving both
* locks+codes fails, the entire operation fails (i.e., does not succeed in acquiring locks
* but fail in acquiring codes).
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct TestRepositoryManager : IRepositoryManager
{
    typedef IBriefcaseManager::ResponseOptions Options;
    typedef IBriefcaseManager::RequestPurpose Purpose;
private:
    Db m_db;

    // impl
    Response _ProcessRequest(Request const& req, DgnDbR db, bool queryOnly) override;
    RepositoryStatus _Demote(DgnLockSet const& locks, DgnCodeSet const& codes, DgnDbR db) override;
    RepositoryStatus _Relinquish(Resources which, DgnDbR db) override;
    RepositoryStatus _QueryHeldResources(DgnLockSet& locks, DgnCodeSet& codes, DgnLockSet& unavailableLocks, DgnCodeSet& unavailableCodes, DgnDbR db) override;
    RepositoryStatus _QueryStates(DgnLockInfoSet&, DgnCodeInfoSet& codeStates, LockableIdSet const& locks, DgnCodeSet const& codes) override;

    DbResult CreateLocksTable();
    DbResult CreateCodesTable();

    // locks
    bool AreLocksAvailable(LockRequestCR reqs, BeBriefcaseId requestor);
    void GetDeniedLocks(DgnLockInfoSet& locks, LockRequestCR reqs, BeBriefcaseId bcId);
    void GetUnavailableLocks(DgnLockSet& locks, BeBriefcaseId bcId);
    int32_t QueryLockCount(BeBriefcaseId bc);
    void Relinquish(DgnLockSet const&, DgnDbR);
    void Reduce(DgnLockSet const&, DgnDbR);
    void _AcquireLocks(Response&, LockRequestCR, DgnDbR, Options opts);
    RepositoryStatus _RelinquishLocks(DgnDbR);
    RepositoryStatus _DemoteLocks(DgnLockSet const&, DgnDbR);
    RepositoryStatus _QueryLocks(DgnLockSet&, DgnDbR);
    RepositoryStatus _QueryLockState(DgnLockInfoR, LockableId);
    RepositoryStatus _QueryLockStates(DgnLockInfoSet&, LockableIdSet const&);

    // codes
    enum class CodeState : uint8_t { Available, Reserved, Discarded, Used };

    RepositoryStatus ValidateRelease(DgnCodeInfoSet& toMarkDiscarded, Statement& stmt, BeBriefcaseId bcId);
    RepositoryStatus ValidateRelease(DgnCodeInfoSet&, DgnCodeSet const&, DgnDbR);
    RepositoryStatus ValidateRelinquish(DgnCodeInfoSet&, DgnDbR);
    void MarkDiscarded(DgnCodeInfoSet const& discarded);
    void MarkRevision(DgnCodeSet const& codes, bool discarded, Utf8StringCR revId);
    void _ReserveCodes(Response& response, DgnCodeSet const& req, DgnDbR db, Options opts, bool queryOnly);
    RepositoryStatus _ReleaseCodes(DgnCodeSet const&, DgnDbR);
    RepositoryStatus _RelinquishCodes(DgnDbR);
    RepositoryStatus _QueryCodeStates(DgnCodeInfoSet&, DgnCodeSet const&);
    void GetUnavailableCodes(DgnCodeSet& codes, BeBriefcaseId bcId);

public:
    TestRepositoryManager();

    // Simulates what the real server does with codes when a revision is pushed.
    void OnFinishRevision(DgnRevision const& rev, DgnDbCR dgndb)
        {
        DgnCodeSet assignedCodes, discardedCodes;
        rev.ExtractCodes(assignedCodes, discardedCodes, dgndb);

        MarkRevision(assignedCodes, false, rev.GetId());
        MarkRevision(discardedCodes, true, rev.GetId());
        }

    void MarkUsed(DgnCode const& code, Utf8StringCR revision)
        {
        DgnCodeSet codes;
        codes.insert(code);
        MarkRevision(codes, false, revision);
        }

    RepositoryStatus _QueryCodes(DgnCodeSet&, DgnDbR);
};

END_BENTLEY_DGN_NAMESPACE

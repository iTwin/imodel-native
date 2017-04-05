/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbRepositoryManager.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/DgnDbRepositoryConnection.h>
#include <DgnPlatform/RepositoryManager.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_WEBSERVICES

typedef RefCountedPtr<struct DgnDbRepositoryManager> DgnDbRepositoryManagerPtr;

//=======================================================================================
//! 
// @bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnDbRepositoryManager : public IRepositoryManager, RefCountedBase
{
private:
    DgnDbRepositoryConnectionPtr m_connection;
    ICancellationTokenPtr        m_cancellationToken;

    Response                     HandleError(Request const& request, DgnDbServerResult<void> result, IBriefcaseManager::RequestPurpose purpose);
    static RepositoryStatus      GetResponseStatus(DgnDbServerResult<void> result);

protected:
    DgnDbRepositoryManager (DgnDbRepositoryConnectionPtr connection) : m_connection(connection) {}

    Response _ProcessRequest(Request const& req, DgnDbR db, bool queryOnly) override;
    RepositoryStatus _Demote(DgnLockSet const& locks, DgnCodeSet const& codes, DgnDbR db) override;
    RepositoryStatus _Relinquish(Resources which, DgnDbR db) override;
    RepositoryStatus _QueryHeldResources(DgnLockSet& locks, DgnCodeSet& codes, DgnLockSet& unavailableLocks, DgnCodeSet& unavailableCodes, DgnDbR db) override;
    RepositoryStatus _QueryStates(DgnLockInfoSet& lockStates, DgnCodeInfoSet& codeStates, LockableIdSet const& locks, DgnCodeSet const& codes) override;

public:
    static DgnDbRepositoryManagerPtr Create(DgnDbRepositoryConnectionPtr connection) {return new DgnDbRepositoryManager(connection);}

    //! Gets used DgnDbRepositoryConnection
    //! @returns DgnDbRepositoryConnection
    DgnDbRepositoryConnectionPtr GetRepositoryConnectionPtr() const {return m_connection;}

    void SetCancellationToken(ICancellationTokenPtr cancellationToken) {m_cancellationToken = cancellationToken;}
};

END_BENTLEY_DGNDBSERVER_NAMESPACE

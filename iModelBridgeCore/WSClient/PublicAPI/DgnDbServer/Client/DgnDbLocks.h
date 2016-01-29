/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbLocks.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/DgnDbRepositoryConnection.h>
#include <DgnPlatform/RepositoryManager.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS
typedef std::shared_ptr<struct DgnDbRepositoryManager> DgnDbRepositoryManagerPtr;
//=======================================================================================
//! 
// @bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct DgnDbRepositoryManager : public IRepositoryManager
    {
private:
    DgnDbRepositoryConnectionPtr m_connection;
    ICancellationTokenPtr m_cancellationToken;
    Credentials m_credentials;
    WebServices::ClientInfoPtr m_clientInfo;

protected:
    virtual Response _ProcessRequest(Request const& req, DgnDbR db) override;
    virtual RepositoryStatus _Demote(DgnLockSet const& locks, DgnCodeSet const& codes, DgnDbR db) override;
    virtual RepositoryStatus _Relinquish(Resources which, DgnDbR db) override;
    virtual RepositoryStatus _QueryHeldResources(DgnLockSet& locks, DgnCodeSet& codes, DgnDbR db) override;
    virtual RepositoryStatus _QueryStates(DgnLockInfoSet& lockStates, DgnCodeInfoSet& codeStates, LockableIdSet const& locks, DgnCodeSet const& codes) override;

    Response _AcquireLocks(LockRequestCR locks, DgnDbR db);
    RepositoryStatus _DemoteLocks(DgnLockSet const& locks, DgnDbR db);
    RepositoryStatus _RelinquishLocks(DgnDbR db);
    RepositoryStatus _QueryLocks(DgnLockSet& locks, DgnDbR db);

    DgnDbRepositoryManager(WebServices::ClientInfoPtr clientInfo);
public:
    static DgnDbRepositoryManagerPtr Create(WebServices::ClientInfoPtr clientInfo);

    DGNDBSERVERCLIENT_EXPORT DgnDbResult Connect(DgnDbCR db);
    DGNDBSERVERCLIENT_EXPORT void SetCancellationToken(ICancellationTokenPtr cancellationToken);
    void SetCredentials(CredentialsCR credentials) { m_credentials = credentials; };
    };
END_BENTLEY_DGNDBSERVER_NAMESPACE

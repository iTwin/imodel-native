/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbRepositoryManager.h $
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
//__PUBLISH_SECTION_END__
private:
    DgnDbRepositoryConnectionPtr m_connection;
    ICancellationTokenPtr        m_cancellationToken;
    Credentials                  m_credentials;
    WebServices::ClientInfoPtr   m_clientInfo;
    AuthenticationHandlerPtr     m_authenticationHandler;

protected:
    DgnDbRepositoryManager (WebServices::ClientInfoPtr clientInfo, AuthenticationHandlerPtr authenticationHandler);

    virtual Response                                _ProcessRequest       (Request const& req, DgnDbR db, bool queryOnly) override;
    virtual RepositoryStatus                        _Demote               (DgnLockSet const& locks, DgnCodeSet const& codes, DgnDbR db) override;
    virtual RepositoryStatus                        _Relinquish           (Resources which, DgnDbR db) override;
    virtual RepositoryStatus                        _QueryHeldResources   (DgnLockSet& locks, DgnCodeSet& codes, DgnLockSet& unavailableLocks, DgnCodeSet& unavailableCodes, DgnDbR db) override;
    virtual RepositoryStatus                        _QueryStates          (DgnLockInfoSet& lockStates, DgnCodeInfoSet& codeStates, LockableIdSet const& locks,
                                                                           DgnCodeSet const& codes) override;
    Response                                        QueryCodesLocksAvailable(Request const& req, DgnDbR db);

public:
    static DgnDbRepositoryManagerPtr                Create                (WebServices::ClientInfoPtr clientInfo, AuthenticationHandlerPtr authenticationHandler = nullptr);
    void                                            SetCredentials        (CredentialsCR credentials) { m_credentials = credentials; };

//__PUBLISH_SECTION_START__
public:
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusResult  Connect               (DgnDbCR db);
    DGNDBSERVERCLIENT_EXPORT void                     SetCancellationToken  (ICancellationTokenPtr cancellationToken);
};

END_BENTLEY_DGNDBSERVER_NAMESPACE

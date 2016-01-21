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
#include <DgnPlatform/LocksManager.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS
typedef std::shared_ptr<struct DgnDbLocks> DgnDbLocksPtr;
//=======================================================================================
//! 
// @bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct DgnDbLocks : public ILocksServer
    {
private:
    DgnDbRepositoryConnectionPtr m_connection;
    ICancellationTokenPtr m_cancellationToken;
    Credentials m_credentials;
    WebServices::ClientInfoPtr m_clientInfo;

protected:
    virtual LockStatus _QueryLocksHeld(bool& held, LockRequestCR locks, DgnDbR db) override;
    virtual LockRequest::Response _AcquireLocks(LockRequestCR locks, DgnDbR db) override;
    virtual LockStatus _DemoteLocks(DgnLockSet const& locks, DgnDbR db) override;
    virtual LockStatus _RelinquishLocks(DgnDbR db) override;
    virtual LockStatus _QueryLockLevels(DgnLockSet&, LockableIdSet const&, DgnDbR db) override;
    virtual LockStatus _QueryLocks(DgnLockSet& locks, DgnDbR db) override;
    virtual LockStatus _QueryOwnerships(DgnOwnedLockSet&, LockableIdSet const&) override;

    DgnDbLocks(WebServices::ClientInfoPtr clientInfo);
public:
    static DgnDbLocksPtr Create(WebServices::ClientInfoPtr clientInfo);

    DGNDBSERVERCLIENT_EXPORT DgnDbResult Connect(DgnDbCR db);
    DGNDBSERVERCLIENT_EXPORT void SetCancellationToken(ICancellationTokenPtr cancellationToken);
    void SetCredentials(CredentialsCR credentials) { m_credentials = credentials; };
    };
END_BENTLEY_DGNDBSERVER_NAMESPACE

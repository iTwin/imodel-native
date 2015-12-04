/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbLocks.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/DgnDbRepositoryConnection.h>
#include <DgnPlatform/LocksManager.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
typedef std::shared_ptr<struct DgnDbLocks> DgnDbLocksPtr;
//=======================================================================================
//! 
// @bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct DgnDbLocks : public Dgn::ILocksServer
    {
private:
    DgnDbRepositoryConnectionPtr m_connection;
    DgnClientFx::Utils::ICancellationTokenPtr m_cancellationToken;
    DgnClientFx::Utils::Credentials m_credentials;
    WebServices::ClientInfoPtr m_clientInfo;

private:
    DgnDbRepositoryConnectionPtr Connect(Dgn::DgnDbCR db);

protected:
    virtual Dgn::LockStatus _QueryLocksHeld(bool& held, Dgn::LockRequestCR locks, Dgn::DgnDbR db) override;
    virtual Dgn::LockRequest::Response _AcquireLocks(Dgn::LockRequestCR locks, Dgn::DgnDbR db) override;
    virtual Dgn::LockStatus _ReleaseLocks(Dgn::DgnLockSet const& locks, Dgn::DgnDbR db) override;
    virtual Dgn::LockStatus _RelinquishLocks(Dgn::DgnDbR db) override;
    virtual Dgn::LockStatus _QueryLockLevel(Dgn::LockLevel& level, Dgn::LockableId lockId, Dgn::DgnDbR db) override;
    virtual Dgn::LockStatus _QueryLocks(Dgn::DgnLockSet& locks, Dgn::DgnDbR db) override;
    virtual Dgn::LockStatus _QueryOwnership(Dgn::DgnLockOwnershipR ownership, Dgn::LockableId lockId, Dgn::DgnDbR db) override;

    DgnDbLocks(WebServices::ClientInfoPtr clientInfo);
public:
    static DgnDbLocksPtr Create(WebServices::ClientInfoPtr clientInfo);
    DGNDBSERVERCLIENT_EXPORT void SetCancellationToken(DgnClientFx::Utils::ICancellationTokenPtr cancellationToken);
    void SetCredentials(DgnClientFx::Utils::CredentialsCR credentials) { m_credentials = credentials; }; //!< Credentials used to authenticate on the server.
    };
END_BENTLEY_DGNDBSERVER_NAMESPACE

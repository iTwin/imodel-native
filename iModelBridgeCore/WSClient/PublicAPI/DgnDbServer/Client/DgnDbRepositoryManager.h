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
USING_NAMESPACE_BENTLEY_WEBSERVICES

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

    Response                     HandleError(Request const& request, DgnDbServerResult<void> result, IBriefcaseManager::RequestPurpose purpose);
    static RepositoryStatus      GetResponseStatus(DgnDbServerResult<void> result);

protected:
    DgnDbRepositoryManager (DgnDbRepositoryConnectionPtr connection);

    virtual Response                                _ProcessRequest       (Request const& req, DgnDbR db, bool queryOnly) override;
    virtual RepositoryStatus                        _Demote               (DgnLockSet const& locks, DgnCodeSet const& codes, DgnDbR db) override;
    virtual RepositoryStatus                        _Relinquish           (Resources which, DgnDbR db) override;
    virtual RepositoryStatus                        _QueryHeldResources   (DgnLockSet& locks, DgnCodeSet& codes, DgnLockSet& unavailableLocks, DgnCodeSet& unavailableCodes, DgnDbR db) override;
    virtual RepositoryStatus                        _QueryStates          (DgnLockInfoSet& lockStates, DgnCodeInfoSet& codeStates, LockableIdSet const& locks,
                                                                           DgnCodeSet const& codes) override;

public:
    static DgnDbRepositoryManagerPtr Create(DgnDbRepositoryConnectionPtr connection);
    //__PUBLISH_SECTION_START__
    DGNDBSERVERCLIENT_EXPORT void                   SetCancellationToken  (ICancellationTokenPtr cancellationToken);
};

END_BENTLEY_DGNDBSERVER_NAMESPACE

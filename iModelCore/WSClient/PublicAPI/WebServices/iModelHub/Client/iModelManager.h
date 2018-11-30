/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Client/iModelManager.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/iModelConnection.h>
#include <DgnPlatform/RepositoryManager.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

USING_NAMESPACE_BENTLEY_WEBSERVICES

typedef RefCountedPtr<struct iModelManager> iModelManagerPtr;

//=======================================================================================
//! 
// @bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE iModelManager : public IRepositoryManager, RefCountedBase
{
private:
    iModelConnectionPtr                     m_connection;
    std::function<ICancellationTokenPtr()>  m_cancellationToken;

protected:
    iModelManager(iModelConnectionPtr connection);

    Response _ProcessRequest(Request const& req, DgnDbR db, bool queryOnly) override;
    RepositoryStatus _Demote(DgnLockSet const& locks, DgnCodeSet const& codes, DgnDbR db) override;
    RepositoryStatus _Relinquish(Resources which, DgnDbR db) override;
    RepositoryStatus _QueryHeldResources(DgnLockSet& locks, DgnCodeSet& codes, DgnLockSet& unavailableLocks, DgnCodeSet& unavailableCodes, 
                                         DgnDbR db) override;
    RepositoryStatus _QueryStates(DgnLockInfoSet& lockStates, DgnCodeInfoSet& codeStates, LockableIdSet const& locks, 
                                  DgnCodeSet const& codes) override;

public:
    static iModelManagerPtr Create(iModelConnectionPtr connection) {return new iModelManager(connection);}

    //! Gets used iModelConnection
    //! @returns iModelConnection
    iModelConnectionPtr GetiModelConnectionPtr() const {return m_connection;}

    void SetCancellationToken(std::function<ICancellationTokenPtr()> cancellationToken) { m_cancellationToken = cancellationToken; };
};
END_BENTLEY_IMODELHUB_NAMESPACE
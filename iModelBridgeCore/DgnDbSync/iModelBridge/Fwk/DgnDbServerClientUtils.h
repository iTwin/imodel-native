/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/DgnDbServerClientUtils.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/RefCounted.h>
#include <Bentley/Tasks/AsyncError.h>
#include <DgnPlatform/DgnPlatform.h>
#include <WebServices/iModelHub/Client/Client.h>
#include <WebServices/iModelHub/Client/Error.h>
#include <BeHttp/Credentials.h>
#include <BeHttp/HttpRequest.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>
#include <WebServices/iModelHub/Client/ClientHelper.h>

BEGIN_BENTLEY_DGN_NAMESPACE

// ========================================================================================================
//! Interface to be adopted by a class the defined the interface between iModelBridgeFwk and iModelHub.
//! This interface define just the services that iModelBridgeFwk requires.
//! This interface is implemented by DgnDbServerClientUtils (below) to provide a real iModelHub connection
//! and by mocks in the test suite to provide controlled stubs.
// ========================================================================================================
struct iModelHubFX
{
    virtual ~iModelHubFX() {}

    //! Sign in to iModel Hub Services via Connect. If sign in succeeds, that moves this object to a valid state.
    //! @return non-zero error status if signin failed.
    //! @param servererror  Optional. If not null, an explanation of sign in failure is returned here.
    //! @param credentials  User credentials.
    //! @see IsSignedIn
    virtual BentleyStatus SignIn(Tasks::AsyncError* servererror, Http::Credentials credentials) = 0;

    //! Look up a BCS project's ID from its name and store that in the connected client. You must call SignIn first.
    //! @param wserror      Optional. If not null, an explanation of query failure is returned here. No details would be returned if the lookup failed simply because the project name was not found.
    //! @param bcsProjectName  The BCS project name to look up
    //! @return non-zero error status if the project ID was not found and stored in the client
    virtual BentleyStatus QueryProjectId(WebServices::WSError* wserror, Utf8StringCR bcsProjectName) = 0;

    //! Call this if you already know the project GUID
    virtual void SetProjectId(Utf8CP guid) = 0;

    //! Query if the client is signed in.
    virtual bool IsSignedIn() const = 0;

//  virtual StatusInt GetRepositories(bvector<DgnDbServer::RepositoryInfoPtr>& repos) = 0;
    virtual StatusInt CreateRepository(Utf8CP repoName, BeFileNameCR localDgnDb) = 0;
    virtual StatusInt AcquireBriefcase(BeFileNameCR bcFileName, Utf8CP repositoryName) = 0;
    virtual StatusInt OpenBriefcase(Dgn::DgnDbR db) = 0;
    virtual StatusInt PullMergeAndPush(Utf8CP) = 0;
    virtual StatusInt PullAndMerge() = 0;
    virtual StatusInt PullAndMergeSchemaRevisions(Dgn::DgnDbPtr& db) = 0;
    virtual iModel::Hub::Error const& GetLastError() const = 0;
    virtual IRepositoryManagerP GetRepositoryManager(DgnDbR db) = 0;
    virtual void CloseBriefcase() = 0;

    virtual StatusInt AcquireLocks(LockRequest&, DgnDbR) = 0;
};

// ========================================================================================================
// Provides an interface to iModelHub.
// ========================================================================================================
struct DgnDbServerClientUtils : iModelHubFX
{
protected:
    iModel::Hub::ClientPtr m_client;
    iModel::Hub::BriefcasePtr m_briefcase;
    iModel::Hub::Error m_lastServerError;
	Utf8String m_projectId;
    uint8_t m_maxRetryCount {};

public:
    DgnDbServerClientUtils(WebServices::UrlProvider::Environment environment, uint8_t nretries);
    ~DgnDbServerClientUtils() {}

    //! Sign in to iModel Hub Services via Connect. If sign in succeeds, that moves this object to a valid state.
    //! @return non-zero error status if signin failed.
    //! @param servererror  Optional. If not null, an explanation of sign in failure is returned here.
    //! @param credentials  User credentials.
    //! @see IsSignedIn
    BentleyStatus SignIn(Tasks::AsyncError* servererror, Http::Credentials credentials) override;
    //! Look up a BCS project's ID from its name and store that in the connected client. You must call SignIn first.
    //! @param wserror      Optional. If not null, an explanation of query failure is returned here. No details would be returned if the lookup failed simply because the project name was not found.
    //! @param bcsProjectName  The BCS project name to look up
    //! @return non-zero error status if the project ID was not found and stored in the client
    BentleyStatus QueryProjectId(WebServices::WSError* wserror, Utf8StringCR bcsProjectName) override;

    //! Call this if you already know the project GUID
    void SetProjectId(Utf8CP guid) override {m_projectId=guid;}

    //! Query if the client is signed in.
    bool IsSignedIn() const override {return m_client.IsValid();}

//    StatusInt GetRepositories(bvector<DgnDbServer::RepositoryInfoPtr>& repos) override;
    StatusInt CreateRepository(Utf8CP repoName, BeFileNameCR localDgnDb) override;
    StatusInt AcquireBriefcase(BeFileNameCR bcFileName, Utf8CP repositoryName) override;
    StatusInt OpenBriefcase(Dgn::DgnDbR db) override;
    StatusInt PullMergeAndPush(Utf8CP) override;
    StatusInt PullAndMerge() override;
    StatusInt PullAndMergeSchemaRevisions(Dgn::DgnDbPtr& db) override;
    iModel::Hub::Error const& GetLastError() const override {return m_lastServerError;}
    IRepositoryManagerP GetRepositoryManager(DgnDbR db) override;
    void CloseBriefcase() override {m_briefcase = nullptr;}

    StatusInt AcquireLocks(LockRequest&, DgnDbR) override;
};

END_BENTLEY_DGN_NAMESPACE

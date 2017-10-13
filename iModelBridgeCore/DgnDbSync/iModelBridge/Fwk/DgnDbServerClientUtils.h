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

struct DgnDbServerClientUtils
{
protected:
    iModel::Hub::ClientPtr m_client;
    iModel::Hub::BriefcasePtr m_briefcase;
    iModel::Hub::Error m_lastServerError;
	Utf8String m_projectId;
    uint8_t m_maxRetryCount {};

public:
    DgnDbServerClientUtils(WebServices::UrlProvider::Environment environment, uint8_t nretries);
    virtual ~DgnDbServerClientUtils() {}

    //! Sign in to iModel Hub Services via Connect. If sign in succeeds, that moves this object to a valid state.
    //! @return non-zero error status if signin failed.
    //! @param servererror  Optional. If not null, an explanation of sign in failure is returned here.
    //! @param credentials  User credentials.
    //! @see IsSignedIn
    virtual BentleyStatus SignIn(Tasks::AsyncError* servererror, Http::Credentials credentials);
    //! Look up a BCS project's ID from its name and store that in the connected client. You must call SignIn first.
    //! @param wserror      Optional. If not null, an explanation of query failure is returned here. No details would be returned if the lookup failed simply because the project name was not found.
    //! @param bcsProjectName  The BCS project name to look up
    //! @return non-zero error status if the project ID was not found and stored in the client
    virtual BentleyStatus QueryProjectId(WebServices::WSError* wserror, Utf8StringCR bcsProjectName);

    //! Call this if you already know the project GUID
    virtual void SetProjectId(Utf8CP guid) {m_projectId=guid;}

    //! Query if the client is signed in.
    virtual bool IsSignedIn() const {return m_client.IsValid();}

//    StatusInt GetRepositories(bvector<DgnDbServer::RepositoryInfoPtr>& repos);
    virtual StatusInt CreateRepository(Utf8CP repoName, BeFileNameCR localDgnDb);
    virtual StatusInt AcquireBriefcase(BeFileNameCR bcFileName, Utf8CP repositoryName);
    virtual StatusInt OpenBriefcase(Dgn::DgnDbR db);
    virtual StatusInt PullMergeAndPush(Utf8CP);
    virtual StatusInt PullAndMerge();
    virtual StatusInt PullAndMergeSchemaRevisions(Dgn::DgnDbPtr& db);
    virtual iModel::Hub::Error const& GetLastError() const {return m_lastServerError;}
    virtual IRepositoryManagerP GetRepositoryManager(DgnDbR db);
    virtual void CloseBriefcase() {m_briefcase = nullptr;}

    virtual StatusInt AcquireLocks(LockRequest&, DgnDbR);
};

END_BENTLEY_DGN_NAMESPACE

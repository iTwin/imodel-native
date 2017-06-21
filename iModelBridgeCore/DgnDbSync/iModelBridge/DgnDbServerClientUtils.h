/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/DgnDbServerClientUtils.h $
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
private:
    iModel::Hub::ClientPtr m_client;
    iModel::Hub::BriefcasePtr m_briefcase;
    iModel::Hub::Error m_lastServerError;

public:
    DgnDbServerClientUtils(WebServices::UrlProvider::Environment environment);

    //! Sign in to iModel Hub Services via Connect. If sign in succeeds, that moves this object to a valid state.
    //! @return non-zero error status if signin failed.
    //! @param servererror  Optional. If not null, an explanation of sign in failure is returned here.
    //! @param credentials  User credentials.
    //! @see IsSignedIn
    BentleyStatus SignIn(Tasks::AsyncError* servererror, Http::Credentials credentials);
    //! Look up a BCS project's ID from its name and store that in the connected client. You must call SignIn first.
    //! @param wserror      Optional. If not null, an explanation of query failure is returned here. No details would be returned if the lookup failed simply because the project name was not found.
    //! @param bcsProjectName  The BCS project name to look up
    //! @return non-zero error status if the project ID was not found and stored in the client
    BentleyStatus QueryProjectId(WebServices::WSError* wserror, Utf8StringCR bcsProjectName);

    //! Query if the client is signed in.
    bool IsSignedIn() const {return m_client.IsValid();}

//    StatusInt GetRepositories(bvector<DgnDbServer::RepositoryInfoPtr>& repos);
    StatusInt CreateRepository(Utf8CP repoName, BeFileNameCR localDgnDb);
    StatusInt AcquireBriefcase(BeFileNameCR bcFileName, Utf8CP repositoryName);
    StatusInt OpenBriefcase(Dgn::DgnDbR db);
    StatusInt PullMergeAndPush(Utf8CP);
    StatusInt PullAndMerge();
    iModel::Hub::Error const& GetLastError() const {return m_lastServerError;}
    IRepositoryManagerP GetRepositoryManager(DgnDbR db);
    void CloseBriefcase() {m_briefcase = nullptr;}

    StatusInt AcquireLocks(LockRequest&, DgnDbR);
};

END_BENTLEY_DGN_NAMESPACE
